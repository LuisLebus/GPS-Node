/* Copyright 2015, ----.
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
 
/* Date: 2015-09-23 */

#ifndef _GPS_NMEA_H_
#define _GPS_NMEA_H_

/*==================[inclusions]=============================================*/
#include "sapi.h"

/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
#define GPS_NMEA_BAUDRATE_4800BITS		4800
#define GPS_NMEA_BAUDRATE_9600BITS		9600
#define GPS_NMEA_BAUDRATE_19200BITS		19200
#define GPS_NMEA_BAUDRATE_115200BITS	115200

#define GPS_NMEA_QUEUE_SIZE		5

/*==================[typedef]================================================*/

typedef struct{
    bool_t valid;
    int32_t latitude;
    int32_t longitude;
    uint8_t speed;
    uint16_t course;
}gpsNmeaRmc_t;

typedef struct {
	uint8_t fix_quality;
    uint8_t satellites_tracked;
}gpsNmeaGga_t;

/*==================[external functions declaration]=========================*/
bool_t gpsNmeaInit(uartMap_t uart, uint32_t baudRate );
bool_t gpsNmeaGetRMC(gpsNmeaRmc_t* frame, uint32_t msToWait);
bool_t gpsNmeaGetGGA(gpsNmeaGga_t* frame, uint32_t msToWait);

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _GPS_NMEA_H_ */
