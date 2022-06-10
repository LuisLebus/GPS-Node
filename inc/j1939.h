/*=====[Module Name]===========================================================
 * Copyright YYYY Author Compelte Name <author@mail.com>
 * All rights reserved.
 * License: license text or at least name and link 
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 0.0.0
 * Creation Date: YYYY/MM/DD
 */

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef _J1939_H_
#define _J1939_H_

/*=====[Inclusions of public function dependencies]==========================*/
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
// J1939 Default Priorities
#define J1939_CONTROL_PRIORITY			0x03
#define J1939_INFO_PRIORITY				0x06
#define J1939_PROPRIETARY_PRIORITY		0x06
#define J1939_REQUEST_PRIORITY			0x06
#define J1939_ACK_PRIORITY				0x06
#define J1939_TP_CM_PRIORITY			0x07
#define J1939_TP_DT_PRIORITY			0x07

// J1939 Defined Addresses
#define J1939_GLOBAL_ADDRESS			255
#define J1939_NULL_ADDRESS				254

// Some J1939 PDU Formats, Control Bytes, and PGN's
#define J1939_PF_REQUEST					234

#define J1939_PGN2_REQ_ADDRESS_CLAIM		0x00
#define J1939_PGN1_REQ_ADDRESS_CLAIM		0xEA
#define J1939_PGN0_REQ_ADDRESS_CLAIM		0x00

#define J1939_PF_ADDRESS_CLAIMED			238		// With global address
#define J1939_PF_CANNOT_CLAIM_ADDRESS		238		// With null address

#define J1939_DATA_LENGTH	8

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/
typedef struct {
	union {
		struct {
			uint8_t sourceAddress;
			uint8_t PDUSpecific;
			uint8_t PDUFormat;
			uint8_t dataPage	:1;
			uint8_t reserved	:1;
			uint8_t priority	:3;
		};
		uint32_t id;
	};
	uint8_t dlc;
	uint8_t data[J1939_DATA_LENGTH];
}j1939Message_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
bool_t j1939Init(canMap_t can, uint8_t filterMode);
bool_t j1939Put(j1939Message_t* j1939Message, uint32_t msToWait);
bool_t j1939Get(j1939Message_t* j1939Message, uint32_t msToWait);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _J1939_H_ */
