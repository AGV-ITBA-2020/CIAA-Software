/*
 * SecuritySystem.hpp
 *
 *  Created on: Nov 22, 2020
 *      Author: Javier
 */
#include "my_sapi.h"
#include "FreeRTOS.h"
#include "GlobalEventGroup.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

extern EventGroupHandle_t xEventGroup;
gpioMap_t emergencyPin=GPIO8;

void emergencyCallback();

void SS_init()
{
	gpioInit( emergencyPin, GPIO_INPUT );
	//Configurar interrupt en 0
}


void emergencyCallback()
{
	xEventGroupSetBitsFromISR( xEventGroup,GEG_EMERGENCY_STOP,NULL );
}
