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

#ifndef _J1939_CONFIG_H_
#define _J1939_CONFIG_H_

/*=====[Inclusions of public function dependencies]==========================*/

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define J1939_STARTING_ADDRESS 		11

#define J1939_ARBITRARY_ADDRESS 	0x00
#define J1939_INDUSTRY_GROUP 		0
#define J1939_VEHICLE_INSTANCE 		0
#define J1939_VEHICLE_SYSTEM 		0
#define J1939_FUNCTION 				2
#define J1939_FUNCTION_INSTANCE 	0
#define J1939_ECU_INSTANCE 			0
#define J1939_MANUFACTURER_CODE 	0
#define J1939_IDENTITY_NUMBER 		50

#define J1939_CA_NAME7 (J1939_ARBITRARY_ADDRESS | (J1939_INDUSTRY_GROUP << 4) | J1939_VEHICLE_INSTANCE)
#define J1939_CA_NAME6 (J1939_VEHICLE_SYSTEM << 1)
#define J1939_CA_NAME5 J1939_FUNCTION
#define J1939_CA_NAME4 ((J1939_FUNCTION_INSTANCE << 3) | J1939_ECU_INSTANCE)
#define J1939_CA_NAME3 (J1939_MANUFACTURER_CODE >> 3)
#define J1939_CA_NAME2 (((J1939_MANUFACTURER_CODE & 0x07) << 5) | (J1939_IDENTITY_NUMBER >> 16))
#define J1939_CA_NAME1 ((J1939_IDENTITY_NUMBER >> 8) & 0xFF)
#define J1939_CA_NAME0 (J1939_IDENTITY_NUMBER & 0xFF)

#define J1939_QUEUE_SIZE		5

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _J1939_CONFIG_H_ */
