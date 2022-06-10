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

#ifndef _DEVICE_H_
#define _DEVICE_H_

/*=====[Inclusions of public function dependencies]==========================*/

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define DEV_J1939_MSG_GPS_RMC_0		0x01
#define DEV_J1939_MSG_GPS_RMC_1		0x02
#define DEV_J1939_MSG_GPS_GGA		0x03
#define DEV_J1939_MSG_IMP_SW		0x04

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/
void deviceInit(void);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* _DEVICE_H_ */
