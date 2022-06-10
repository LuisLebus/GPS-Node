/*=====[Module Name]===========================================================
 * Copyright YYYY Author Compelte Name <author@mail.com>
 * All rights reserved.
 * License: license text or at least name and link 
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Creation Date: YYYY/MM/DD
 */
 
/*=====[Inclusion of own header]=============================================*/
#include <string.h>

#include "j1939.h"
#include "j1939Config.h"
#include "sapi.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/
static uint8_t j1939CaName[J1939_DATA_LENGTH];
static uint8_t j1939Address = J1939_STARTING_ADDRESS;
static bool_t j1939WaitAddressClaimContention = true;

static xQueueHandle j1939RxQueue = NULL;
static xQueueHandle j1939TxQueue = NULL;

canMap_t canUsed = CAN2;

/*=====[Prototypes (declarations) of private functions]======================*/
static void j1939TxTask(void *pvParameters);
static void j1939RxTask(void *pvParameters);
static void j1939AddressClaimTask(void *pvParameters);
static int8_t j1939CompareNames(uint8_t* name);
static void j1939SendAddressClaim(void);
static void j1939TxAddressClaimHandling(void);
static void j1939RxAddressClaimHandling(j1939Message_t* j1939Message);

/*=====[Implementations of public functions]=================================*/
bool_t j1939Init(canMap_t can, bool_t softFilter)
{
	bool_t res;
	portBASE_TYPE resTask;
	canUsed = can;

	res = canInit(canUsed, CAN_BAUDRATE_250KBITS);
	if(res == false)
		return false;

	canCallbackSet(canUsed, CAN_RECEIVE, 0, NULL);
	canInterrupt(canUsed, true);

	if(softFilter)
		canDisableFilter(canUsed);

	j1939CaName[7] = J1939_CA_NAME7;
	j1939CaName[6] = J1939_CA_NAME6;
	j1939CaName[5] = J1939_CA_NAME5;
	j1939CaName[4] = J1939_CA_NAME4;
	j1939CaName[3] = J1939_CA_NAME3;
	j1939CaName[2] = J1939_CA_NAME2;
	j1939CaName[1] = J1939_CA_NAME1;
	j1939CaName[0] = J1939_CA_NAME0;

	j1939RxQueue = xQueueCreate(J1939_QUEUE_SIZE, sizeof(j1939Message_t));
	if(j1939RxQueue == NULL)
		return false;

	j1939TxQueue = xQueueCreate(J1939_QUEUE_SIZE, sizeof(j1939Message_t));
	if(j1939TxQueue == NULL)
		return false;

	resTask = xTaskCreate(j1939TxTask, (signed char *) "j1939TxTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
	if(resTask != pdPASS)
		return false;

	resTask = xTaskCreate(j1939RxTask, (signed char *) "j1939RxTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
	if(resTask != pdPASS)
		return false;

	j1939TxAddressClaimHandling();

	return true;
}

bool_t j1939Put(j1939Message_t* j1939Message, uint32_t msToWait)
{
	bool_t retVal = false;

	if(j1939WaitAddressClaimContention == 0)
	{
		if( xQueueSend(j1939TxQueue, j1939Message, msToWait / portTICK_RATE_MS) == pdTRUE)
			retVal = true;
	}

	return retVal;
}

bool_t j1939Get(j1939Message_t* j1939Message, uint32_t msToWait)
{
	bool_t retVal = false;

	if( xQueueReceive(j1939RxQueue, j1939Message, msToWait / portTICK_RATE_MS) == pdTRUE)
		retVal = true;

	return retVal;
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void j1939TxTask(void *pvParameters)
{
	canMessage_t canMessage;
	j1939Message_t j1939Message;

	while (1)
	{
		if( xQueueReceive(j1939TxQueue, &j1939Message, portMAX_DELAY) == pdTRUE)
		{
			j1939Message.sourceAddress = j1939Address;

			canMessage.id = j1939Message.id;
			canMessage.idType = CAN_EXTENDED_ID;
			canMessage.dlc = j1939Message.dlc;
			memcpy(canMessage.data, j1939Message.data, j1939Message.dlc);

			canPut(canUsed, &canMessage, 0);
		}
	}
}

static void j1939RxTask(void *pvParameters)
{
	canMessage_t canMessage;
	j1939Message_t j1939Message;

	while (1)
	{
		if( canGet(canUsed, &canMessage, portMAX_DELAY) )
		{
			j1939Message.id = canMessage.id;
			j1939Message.dlc = canMessage.dlc;
			memcpy(j1939Message.data, canMessage.data, canMessage.dlc);

			if( ( (j1939Message.PDUSpecific != J1939_NULL_ADDRESS) && (j1939Message.PDUSpecific == j1939Address) )
				|| j1939Message.PDUSpecific == J1939_GLOBAL_ADDRESS)
			{
				switch( j1939Message.PDUFormat )
				{
					case J1939_PF_REQUEST:
						if( (j1939Message.data[0] == J1939_PGN0_REQ_ADDRESS_CLAIM) && (j1939Message.data[1] == J1939_PGN1_REQ_ADDRESS_CLAIM) && (j1939Message.data[2] == J1939_PGN2_REQ_ADDRESS_CLAIM) )
							j1939SendAddressClaim();
						break;
					case J1939_PF_ADDRESS_CLAIMED:
						j1939RxAddressClaimHandling(&j1939Message);
						break;
					default:
						xQueueSend(j1939RxQueue, &j1939Message, 0);
				}
			}
		}
	}
}

static void j1939AddressClaimTask(void *pvParameters)
{
	vTaskDelay(250 / portTICK_RATE_MS);

	if(j1939WaitAddressClaimContention)
		j1939WaitAddressClaimContention = 0;

	vTaskDelete(NULL);
}

static int8_t j1939CompareNames(uint8_t* name)
{
	int8_t retVal;
	uint8_t i;

	for(i=0; (i<J1939_DATA_LENGTH) && (name[i] == j1939CaName[i]); i++)
	{

	}

	if(i == J1939_DATA_LENGTH)
		retVal = 0;
	else if(j1939CaName[i] < name[i] )
		retVal = -1;
	else
		retVal = 1;

	return retVal;
}

static void j1939SendAddressClaim(void)
{
	j1939Message_t j1939Message;

	j1939Message.sourceAddress = j1939Address;
	j1939Message.PDUFormat = J1939_PF_ADDRESS_CLAIMED;	// Same as J1939_PF_CANNOT_CLAIM_ADDRESS
	j1939Message.PDUSpecific = J1939_GLOBAL_ADDRESS;
	j1939Message.priority = J1939_CONTROL_PRIORITY;
	j1939Message.dlc = J1939_DATA_LENGTH;
	memcpy(j1939Message.data, j1939CaName, J1939_DATA_LENGTH);

	xQueueSend(j1939TxQueue, &j1939Message, 0);
}

static void j1939TxAddressClaimHandling(void)
{
	j1939SendAddressClaim();

	j1939WaitAddressClaimContention = true;
	xTaskCreate(j1939AddressClaimTask, (signed char *) "j1939AddressClaimTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
}

static void j1939RxAddressClaimHandling(j1939Message_t* j1939Message)
{
	if( (j1939Message->sourceAddress == j1939Address) && (j1939Message->sourceAddress != J1939_NULL_ADDRESS) )
	{
		if( j1939CompareNames(j1939Message->data) != -1 ) // Our CA_Name is not less
		{
			// Send Cannot Claim Address message
			j1939Address = J1939_NULL_ADDRESS;

			j1939Message->sourceAddress = j1939Address;
			j1939Message->PDUFormat = J1939_PF_ADDRESS_CLAIMED;
			j1939Message->PDUSpecific = J1939_GLOBAL_ADDRESS;
			j1939Message->priority = J1939_CONTROL_PRIORITY;
			j1939Message->dlc = J1939_DATA_LENGTH;
			memcpy(j1939Message->data, j1939CaName, J1939_DATA_LENGTH);

			xQueueSend(j1939TxQueue, j1939Message, 0);

			j1939WaitAddressClaimContention = 0;
		}
		else
		{
			j1939TxAddressClaimHandling();
		}
	}
}
