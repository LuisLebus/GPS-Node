/*=============================================================================
 * Author: Luis Lebus
 * Date: 2019/09/04
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "sapi.h"
#include "device.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	// ----- Setup -----------------------------------
	boardInit();

	deviceInit();

	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the
	// case of a PC program.
	return 0;
}
