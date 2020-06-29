/*
 * my_sapi.c
 *
 *  Created on: 13 jun. 2020
 *      Author: mdevo
 */

#include "my_sapi.h"
#include "board.h"

void MySapi_BoardInit(bool_t debugUartEnable)
{
	if(debugUartEnable)
		DEBUGINIT();
	SystemCoreClockUpdate();
}
