/* Copyright 2015, ---.
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

/* Date: 2015-09-23 */

/*==================[inclusions]=============================================*/
#include "gps_nmea.h"
#include "minmea.h"
#include "sapi.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*==================[macros and definitions]=================================*/
#define GPS_NMEA_MAX_LENGHT	MINMEA_MAX_LENGTH

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
static void gpsNmeaOnRx(void* param);

/*==================[internal data definition]===============================*/
static uartMap_t uartUsed = 0;

static xQueueHandle gpsNmeaRmcQueue;
static xQueueHandle gpsNmeaGgaQueue;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static void gpsNmeaOnRx(void* param)
{
	enum minmea_sentence_id frameReceived = MINMEA_UNKNOWN;
	static volatile char gpsNmeaLine[GPS_NMEA_MAX_LENGHT] = {0};
	static volatile uint8_t i = 0;
	char val = 0;

	struct minmea_sentence_rmc sentenceRmc;
	struct minmea_sentence_gga sentenceGga;

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	val = uartRxRead(uartUsed);

	if(val == '$')
	{
		i = 0;
		gpsNmeaLine[i] = val;
		i++;
	}
	else
	{
		gpsNmeaLine[i] = val;

		if( ( (val == '\n') || (val == '\r') ) && (gpsNmeaLine[0] == '$') )
		{
			gpsNmeaLine[i + 1] = 0;

			frameReceived = minmea_sentence_id( (const char*)gpsNmeaLine, true );

			if(frameReceived == MINMEA_SENTENCE_RMC)
			{
				if( minmea_parse_rmc(&sentenceRmc, (const char*)gpsNmeaLine) )
				{
					xQueueSendFromISR(gpsNmeaRmcQueue, &sentenceRmc, &xHigherPriorityTaskWoken);
				}
			}
			else if(frameReceived == MINMEA_SENTENCE_GGA)
			{
				if( minmea_parse_gga(&sentenceGga, (const char*)gpsNmeaLine) )
				{
					xQueueSendFromISR(gpsNmeaGgaQueue, &sentenceGga, &xHigherPriorityTaskWoken);
				}
			}
		}

		if(i < (GPS_NMEA_MAX_LENGHT - 1))
			i++;
		else
			i = 0;
	}
}

/*==================[external functions definition]==========================*/
bool_t gpsNmeaInit(uartMap_t uart, uint32_t baudRate )
{
	uartUsed = uart;

	uartInit(uartUsed, baudRate);
	uartCallbackSet(uartUsed, UART_RECEIVE, gpsNmeaOnRx, NULL);
	uartInterrupt(uartUsed, true);

	gpsNmeaRmcQueue = xQueueCreate(GPS_NMEA_QUEUE_SIZE, sizeof(struct minmea_sentence_rmc));
	if(gpsNmeaRmcQueue == NULL)
		return false;

	gpsNmeaGgaQueue = xQueueCreate(GPS_NMEA_QUEUE_SIZE, sizeof(struct minmea_sentence_gga));
	if(gpsNmeaGgaQueue == NULL)
		return false;

	return true;
}

bool_t gpsNmeaGetRMC(gpsNmeaRmc_t* gpsNmeaRmc, uint32_t msToWait)
{
	bool_t retVal = false;
	struct minmea_sentence_rmc sentenceRmc;

	if( xQueueReceive(gpsNmeaRmcQueue, &sentenceRmc, msToWait / portTICK_RATE_MS) == pdTRUE)
	{
		gpsNmeaRmc->latitude = (int32_t)(minmea_tocoord(&sentenceRmc.latitude) * 1000000);
		gpsNmeaRmc->longitude = (int32_t)(minmea_tocoord(&sentenceRmc.longitude) * 1000000);
		gpsNmeaRmc->speed = (uint8_t)minmea_tofloat(&sentenceRmc.speed);
		gpsNmeaRmc->valid = sentenceRmc.valid;
		gpsNmeaRmc->course = (uint16_t)minmea_tofloat(&sentenceRmc.course);

		retVal = true;
	}

	return retVal;
}

bool_t gpsNmeaGetGGA(gpsNmeaGga_t* gpsNmeaGga, uint32_t msToWait)
{
	bool_t retVal = false;
	struct minmea_sentence_gga sentenceGga;


	if( xQueueReceive(gpsNmeaGgaQueue, &sentenceGga, msToWait / portTICK_RATE_MS) == pdTRUE)
	{
		gpsNmeaGga->fix_quality = sentenceGga.fix_quality;
		gpsNmeaGga->satellites_tracked = sentenceGga.satellites_tracked;

		retVal = true;
	}

	return retVal;
}

/*==================[end of file]============================================*/
