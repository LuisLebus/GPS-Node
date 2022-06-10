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
#include "FreeRTOS.h"
#include "task.h"

#include "device.h"
#include "sapi.h"
#include "gps_nmea.h"
#include "j1939.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/
static void devJ1939RxTask(void *pvParameters);
static void devGpsNmeaRxTask(void *pvParameters);

/*=====[Implementations of public functions]=================================*/
void deviceInit(void)
{
	gpsNmeaInit(UART0, GPS_NMEA_BAUDRATE_4800BITS);
	j1939Init(CAN2, true);

	xTaskCreate(devJ1939RxTask, (signed char *) "devJ1939RxTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
	xTaskCreate(devGpsNmeaRxTask, (signed char *) "devGpsNmeaRxTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();
}
/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/
static void devJ1939RxTask(void *pvParameters)
{
	j1939Message_t j1939Message;

	while (1)
	{
		if( j1939Get(&j1939Message, portMAX_DELAY) )
		{
			//Este dispositivo descarta todos los mensajes recibidos
		}
	}
}

static void devGpsNmeaRxTask(void *pvParameters)
{
	j1939Message_t j1939Message;
	gpsNmeaGga_t sentenceGga;
	gpsNmeaRmc_t sentenceRmc;

	while (1)
	{
		if( gpsNmeaGetGGA(&sentenceGga, 0) )
		{
			j1939Message.PDUFormat = DEV_J1939_MSG_GPS_GGA;
			j1939Message.PDUSpecific = J1939_GLOBAL_ADDRESS;
			j1939Message.priority = J1939_INFO_PRIORITY;
			j1939Message.dlc = J1939_DATA_LENGTH;
			j1939Message.data[0] = sentenceGga.fix_quality;
			j1939Message.data[1] = sentenceGga.satellites_tracked;
			j1939Message.data[2] = 0;
			j1939Message.data[3] = 0;
			j1939Message.data[4] = 0;
			j1939Message.data[5] = 0;
			j1939Message.data[6] = 0;
			j1939Message.data[7] = 0;

			j1939Put(&j1939Message, 0);
		}

		if( gpsNmeaGetRMC(&sentenceRmc, 0) )
		{
			j1939Message.PDUFormat = DEV_J1939_MSG_GPS_RMC_0;
			j1939Message.PDUSpecific = J1939_GLOBAL_ADDRESS;
			j1939Message.priority = J1939_INFO_PRIORITY;
			j1939Message.dlc = J1939_DATA_LENGTH;
			*(int32_t*)j1939Message.data = sentenceRmc.latitude;
			*(int32_t*)(j1939Message.data + 4) = sentenceRmc.longitude;

			j1939Put(&j1939Message, 0);

			j1939Message.PDUFormat = DEV_J1939_MSG_GPS_RMC_1;
			j1939Message.PDUSpecific = J1939_GLOBAL_ADDRESS;
			j1939Message.priority = J1939_INFO_PRIORITY;
			j1939Message.dlc = J1939_DATA_LENGTH;
			j1939Message.data[0] = sentenceRmc.speed;
			j1939Message.data[1] = sentenceRmc.valid;
			*(uint16_t*)(j1939Message.data + 2) = sentenceRmc.course;
			j1939Message.data[4] = 0;
			j1939Message.data[5] = 0;
			j1939Message.data[6] = 0;
			j1939Message.data[7] = 0;

			j1939Put(&j1939Message, 0);
		}

		taskYIELD();
	}
}
