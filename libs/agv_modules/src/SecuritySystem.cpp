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
		if(read==1 && lastEmergSignalValue)
			xEventGroupSetBits( xEventGroup,GEG_EMERGENCY_STOP);
		lastEmergSignalValue=read;
		vTaskDelayUntil(&xLastTimeWoke, hmiUpdatePeriod);
	}

}


void SS_init()
{
	gpioInit( emergencyPin, GPIO_INPUT );
	xTaskCreate(SS_MainTask, "HMI_TASK", 100, NULL, 1, NULL);
	//Habría que hacerlo con callbacks pero una paja de leer eso, no hay nada hecho
}


void emergencyCallback()
{
	xEventGroupSetBitsFromISR( xEventGroup,GEG_EMERGENCY_STOP,NULL );
}
