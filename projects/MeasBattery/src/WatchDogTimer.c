/*
 * WatchDogTimer.c
 *
 *  Created on: 19 dic. 2020
 *      Author: mdevo
 */

#include "WatchDogTimer.h"
#include "my_sapi.h"
#include "wwdt_18xx_43xx.h"

void WDT_Start()
{
	Chip_WWDT_Init(LPC_WWDT);
	Chip_WWDT_SetTimeOut(LPC_WWDT, (uint32_t)(2*WDT_OSC));	// Watchdog set for 2 second
	Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDRESET);	// WWDT timeout will cause reset
	Chip_WWDT_Start(LPC_WWDT);
}

bool_t WDT_HasTimeout()
{
	if( (Chip_WWDT_GetStatus(LPC_WWDT) & WWDT_WDMOD_WDTOF) )
	{
		Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF);	// Clear flag
		return true;
	}
	else
		return false;
}

uint32_t WDT_GetStatus()
{
	return Chip_WWDT_GetStatus(LPC_WWDT);
}

void WDT_Feed()
{
	Chip_WWDT_Feed(LPC_WWDT);
}

uint32_t WDT_GetCount()
{
	return Chip_WWDT_GetCurrentCount(LPC_WWDT);
}
