/* Copyright 2015, ---
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
 */

/* Date: 2016-02-26 */

#ifndef _SAPI_CAN_H_
#define _SAPI_CAN_H_

/*==================[inclusions]=============================================*/
#include "sapi.h"

/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
#define CAN_BAUDRATE_100KBITS	100000
#define CAN_BAUDRATE_125KBITS	125000
#define CAN_BAUDRATE_250KBITS	250000
#define CAN_BAUDRATE_500KBITS	500000

/*==================[typedef]================================================*/
typedef enum {
   CAN_STANDARD_ID = 0,
   CAN_EXTENDED_ID
}canIdType_t;

typedef struct {
	canIdType_t idType;
	uint32_t id;
	uint8_t dlc;
	uint8_t data[8];
}canMessage_t;

typedef enum{
   CAN_RECEIVE = 0
}canEvents_t;

/*==================[external functions declaration]=========================*/
bool_t canInit(canMap_t can, uint32_t baudRate);
bool_t canPut(canMap_t can, canMessage_t* canMessage, uint32_t msToWait);
bool_t canGet(canMap_t can, canMessage_t* canMessage, uint32_t msToWait);
void canDisableFilter(canMap_t can);
void canSetFilter(canMap_t can, uint32_t filter, canIdType_t type);
void canClearFilter(canMap_t can, uint32_t filter, canIdType_t type);

#ifdef SAPI_USE_INTERRUPTS
//-------------------------------------------------------------
// Interrupts
//-------------------------------------------------------------

// CAN Global Interrupt Enable/Disable
void canInterrupt(canMap_t can, bool_t enable);

// CAN Interrupt event Enable and set a callback
void canCallbackSet(canMap_t can, canEvents_t event, callBackFuncPtr_t callbackFunc, void* callbackParam);

// CAN Interrupt event Disable
void canCallbackClr(canMap_t can, canEvents_t event);

/*==================[ISR external functions declaration]======================*/
void CAN_IRQHandler(void);

#endif /* SAPI_USE_INTERRUPTS */

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _SAPI_CAN_H_ */
