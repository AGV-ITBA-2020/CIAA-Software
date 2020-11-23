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

#include "HMIWrapper.hpp"

extern EventGroupHandle_t xEventGroup;
gpioMap_t emergencyPin=GPIO8;

void emergencyCallback();
void SS_MainTask(void *);

void SS_MainTask(void *)
{
	// Set initial variables
	TickType_t xLastTimeWoke = xTaskGetTickCount();
	TickType_t hmiUpdatePeriod = pdMS_TO_TICKS(50);
	bool_t lastEmergSignalValue=0;
	for( ;; )
	{
		bool_t read=gpioRead(emergencyPin);
		if(read && !lastEmergSignalValue){
			xEventGroupSetBits( xEventGroup,GEG_EMERGENCY_STOP);
			HMIW_SetOutput(OUTPUT_LEDSTRIP_STOP, true);
		}
		else if (!read && lastEmergSignalValue)
		{
			HMIW_SetOutput(OUTPUT_LEDSTRIP_STOP, false);
		}
		lastEmergSignalValue=read;
		vTaskDelayUntil(&xLastTimeWoke, hmiUpdatePeriod);
	}

}


void SS_init()
{
	gpioInit( emergencyPin, GPIO_INPUT );
	xTaskCreate(SS_MainTask, "HMI_TASK", 100, NULL, 1, NULL);
	//Habr�a que hacerlo con callbacks pero una paja de leer eso, no hay nada hecho
}


void emergencyCallback()
{
	xEventGroupSetBitsFromISR( xEventGroup,GEG_EMERGENCY_STOP,NULL );
}

bool_t SS_emergencyState()
{
	return gpioRead(emergencyPin);
}
