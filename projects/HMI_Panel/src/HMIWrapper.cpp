/*
 * HMIWrapper.cpp
 *
 *  Created on: Nov 22, 2020
 *      Author: Javier
 */

#include "HMIWrapper.hpp"
#include "GlobalEventGroup.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"

extern EventGroupHandle_t xEventGroup;
/*==================[internal functions definition]==========================*/

/*==================[external functions declaration]=========================*/


void HMIW_Init()
{
	HMI_Init();
}
/*==================[ callbacks]==========================*/
