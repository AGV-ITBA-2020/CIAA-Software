/*
 * testMC.c
 *
 *  Created on: Sep 17, 2020
 *      Author: Sebastian
 */

#include "MovementControlModule.h"


int main(void)
{
	MySapi_BoardInit(true);
	MC_Init();
	vTaskStartScheduler();

	return 0;
}
