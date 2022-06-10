/* Copyright ---.
 * Copyright ---.
 * Copyright ---.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Date: 2016-02-26 */

/*==================[inclusions]=============================================*/
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sapi_can.h"

/*==================[macros]=================================================*/
#define CAN_QUEUE_SIZE		5

/*==================[typedef]================================================*/
typedef struct {
	LPC_CAN_T* 		addr;
	pinConfig_t		tx;
	pinConfig_t		rx;
	IRQn_Type       irqAddr;
} canModule_t;

typedef struct {
	callBackFuncPtr_t 	rxIsrCallback;
	void* 				rxIsrCallbackParams;
} canCallback_t;

/*==================[internal data declaration]==============================*/
static const canModule_t canModule[CAN_MAX] = {
		// { canAddr, { txPort, txpin, txfunc }, { rxPort, rxpin, rxfunc }, canIrqAddr },

		// CAN2 (GPIO0_5 = TX, GPIO0_4 = RX)
		{ LPC_CAN2, { FUNC2, 0, 5 }, { FUNC2, 0, 4 }, CAN_IRQn },	// CAN2

};

static canCallback_t canCallback[CAN_MAX] = {
		// { rxCallback, rxCallbackParam },
		{ 0, NULL },	// CAN0
};

static xQueueHandle canRxQueue[CAN_MAX] = { NULL };
static xQueueHandle canTxQueue[CAN_MAX] = { NULL };

/*==================[internal functions declaration]=========================*/
#ifdef SAPI_USE_INTERRUPTS
static void canProcessIRQ(void);

/*==================[internal functions definition]==========================*/
static void canProcessIRQ(void)
{
	CAN_MSG_T RcvMsgBuf;
	canMessage_t canMessage;
	uint32_t status;
	uint8_t i;
	portBASE_TYPE xHigherPriorityTaskWoken[CAN_MAX];

	for(i=0; i<CAN_MAX; i++)
	{
		status = Chip_CAN_GetIntStatus(canModule[i].addr);

		if(status & CAN_ICR_RI)
		{
			Chip_CAN_Receive(canModule[i].addr, &RcvMsgBuf);

			canMessage.dlc = RcvMsgBuf.DLC;
			memcpy(&canMessage.data, &RcvMsgBuf.Data, canMessage.dlc);

			if(RcvMsgBuf.ID & CAN_EXTEND_ID_USAGE)
			{
				canMessage.idType = CAN_EXTENDED_ID;
				canMessage.id = RcvMsgBuf.ID & 0x1FFFFFFF;	//29 bits
			}
			else
			{
				canMessage.idType = CAN_STANDARD_ID;
				canMessage.id = RcvMsgBuf.ID & 0x3FF;		//11 bits
			}

			if(canRxQueue[i] != NULL)
			{
				xHigherPriorityTaskWoken[i] = pdFALSE;
				xQueueSendFromISR(canRxQueue[i], &canMessage, &xHigherPriorityTaskWoken[i]);
			}

			// Execute callback
			if(canCallback[i].rxIsrCallback != 0)
				(*canCallback[i].rxIsrCallback)(0);
		}
	}

	for(i=0; i<CAN_MAX; i++)
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken[i] );
		break;
	}
}
#endif /* SAPI_USE_INTERRUPTS */



static void canTxTask(void *pvParameters)
{
	canMessage_t canMessage;
	CAN_BUFFER_ID_T TxBuf;
	CAN_MSG_T SendMsgBuf = {0};
	uint8_t i;

	while (1)
	{
		for(i=0; i<CAN_MAX; i++)
		{
			if(canTxQueue[i] != NULL)
			{
				if( xQueueReceive(canTxQueue[i], &canMessage, 0) == pdTRUE)
				{
					SendMsgBuf.DLC = canMessage.dlc;
					memcpy(&SendMsgBuf.Data, &canMessage.data, SendMsgBuf.DLC);
					SendMsgBuf.ID = canMessage.id;

					if(canMessage.idType == CAN_EXTENDED_ID)
						SendMsgBuf.ID |= CAN_EXTEND_ID_USAGE;

					SendMsgBuf.Type = 0;

					TxBuf = Chip_CAN_GetFreeTxBuf(canModule[i].addr);
					Chip_CAN_Send(canModule[i].addr, TxBuf, &SendMsgBuf);
				}
			}
		}

		taskYIELD();
	}
}

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
#ifdef SAPI_USE_INTERRUPTS

// CAN Global Interrupt Enable/Disable
void canInterrupt( canMap_t can, bool_t enable )
{
	if(enable)
	{
		// Interrupt Priority for CAN channel
		NVIC_SetPriority(canModule[can].irqAddr, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY); // FreeRTOS Requiere prioridad >= 5 (numero mas alto, mas baja prioridad)
		// Enable Interrupt for CAN channel
		NVIC_EnableIRQ(canModule[can].irqAddr);
	}
	else
	{
		// Disable Interrupt for CAN channel
		NVIC_DisableIRQ(canModule[can].irqAddr);
	}
}

// CAN Interrupt event Enable and set a callback
void canCallbackSet(canMap_t can, canEvents_t event, callBackFuncPtr_t callbackFunc, void* callbackParam)
{   
	uint32_t intMask = 0;

	switch(event)
	{
	case CAN_RECEIVE:
		// Enable CAN Receiver Buffer Register Interrupt
		intMask |= CAN_IER_RIE;

		if(callbackFunc != 0)
		{
			// Set callback
			canCallback[can].rxIsrCallback = callbackFunc;
			canCallback[can].rxIsrCallbackParams = callbackParam;
		}

		// Enable CAN Interrupt
		Chip_CAN_EnableInt(canModule[can].addr, intMask);

		break;
	}

}

// CAN Interrupt event Disable
void canCallbackClr(canMap_t can, canEvents_t event)
{
	uint32_t intMask = 0;

	switch(event)
	{
	case CAN_RECEIVE:
		// Enable CAN Receiver Buffer Register Interrupt
		intMask |= CAN_IER_RIE;

		// Set callback
		canCallback[can].rxIsrCallback = 0;
		canCallback[can].rxIsrCallbackParams = NULL;

		// Disable CAN Interrupt
		Chip_CAN_DisableInt(canModule[can].addr, intMask);
		break;
	}
}

#endif /* SAPI_USE_INTERRUPTS */


bool_t canInit(canMap_t can, uint32_t baudRate)
{
	portBASE_TYPE resTask;

	// Configure CANn_TXD uartPin
	Chip_IOCON_PinMux(
			LPC_IOCON,
			canModule[can].tx.port,
			canModule[can].tx.pin,
			IOCON_MODE_INACT,
			canModule[can].tx.func );

	// Configure CANn_RXD uartPin
	Chip_IOCON_PinMux(
			LPC_IOCON,
			canModule[can].rx.port,
			canModule[can].rx.pin,
			IOCON_MODE_INACT,
			canModule[can].rx.func );

	Chip_CAN_Init(canModule[can].addr, LPC_CANAF, LPC_CANAF_RAM);
	Chip_CAN_SetBitRate(canModule[can].addr, baudRate);

	//Habilita modo self test
	//Chip_CAN_SetMode(canModule[can].addr, CAN_MOD_STM, ENABLE);

	canRxQueue[can] = xQueueCreate(CAN_QUEUE_SIZE, sizeof(canMessage_t));
	if(canRxQueue[can] == NULL)
		return false;

	canTxQueue[can] = xQueueCreate(CAN_QUEUE_SIZE, sizeof(canMessage_t));
	if(canTxQueue[can] == NULL)
		return false;

	resTask = xTaskCreate(canTxTask, (signed char *) "canTxTask", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (xTaskHandle *) NULL);
	if(resTask != pdPASS)
		return false;

	return true;
}

bool_t canPut(canMap_t can, canMessage_t* canMessage, uint32_t msToWait)
{
	bool_t retVal = false;

	if( xQueueSend(canTxQueue[can], canMessage, msToWait / portTICK_RATE_MS) == pdTRUE)
		retVal = true;

	return retVal;
}

bool_t canGet(canMap_t can, canMessage_t* canMessage, uint32_t msToWait)
{
	bool_t retVal = false;

	if( xQueueReceive(canRxQueue[can], canMessage, msToWait / portTICK_RATE_MS) == pdTRUE)
		retVal = true;

	return retVal;
}

void canSetFilter(canMap_t can, uint32_t filter, canIdType_t type)
{
	CAN_STD_ID_ENTRY_T StdEntry;
	CAN_EXT_ID_ENTRY_T ExtEntry;

	if(type == CAN_EXTENDED_ID)
	{
		ExtEntry.CtrlNo = can;
		ExtEntry.ID_29 = filter;
		Chip_CAN_InsertEXTEntry(LPC_CANAF, LPC_CANAF_RAM, &ExtEntry);
	}
	else
	{
		StdEntry.CtrlNo = can;
		StdEntry.Disable = 0;
		StdEntry.ID_11 = filter;
		Chip_CAN_InsertSTDEntry(LPC_CANAF, LPC_CANAF_RAM, &StdEntry);
	}
}

void canClearFilter(canMap_t can, uint32_t filter, canIdType_t type)
{
	CAN_STD_ID_ENTRY_T StdEntry;
	CAN_EXT_ID_ENTRY_T ExtEntry;

	uint16_t i, entriesNum;

	if(type == CAN_EXTENDED_ID)
	{
		entriesNum = Chip_CAN_GetEntriesNum(LPC_CANAF, LPC_CANAF_RAM, CANAF_RAM_EFF_SEC);

		for(i=0; i<entriesNum; i++)
		{
			Chip_CAN_ReadEXTEntry(LPC_CANAF, LPC_CANAF_RAM, i, &ExtEntry);
			if( (ExtEntry.ID_29 == filter) && (ExtEntry.CtrlNo == can) )
			{
				Chip_CAN_RemoveEXTEntry(LPC_CANAF, LPC_CANAF_RAM, i);
				break;
			}
		}
	}
	else
	{
		entriesNum = Chip_CAN_GetEntriesNum(LPC_CANAF, LPC_CANAF_RAM, CANAF_RAM_SFF_SEC);

		for(i=0; i<entriesNum; i++)
		{
			Chip_CAN_ReadSTDEntry(LPC_CANAF, LPC_CANAF_RAM, i, &StdEntry);
			if( (StdEntry.ID_11 == filter) && (StdEntry.CtrlNo == can) )
			{
				Chip_CAN_RemoveSTDEntry(LPC_CANAF, LPC_CANAF_RAM, i);
				break;
			}
		}
	}
}

void canDisableFilter(canMap_t can)
{
	Chip_CAN_SetAFMode(LPC_CANAF, CAN_AF_BYBASS_MODE);
}

/*==================[ISR external functions definition]======================*/

#ifdef SAPI_USE_INTERRUPTS

__attribute__ ((section(".after_vectors")))

void CAN_IRQHandler(void)
{
	canProcessIRQ();
}
#endif /* SAPI_USE_INTERRUPTS */

/*==================[end of file]============================================*/
