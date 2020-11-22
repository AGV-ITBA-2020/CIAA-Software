/*
 * HMIWTest.cpp
 *
 *  Created on: Nov 22, 2020
 *      Author: Javier
 */

#include "config.h"        // <= Biblioteca sAPI
#include "HMIWrapper.hpp"
#include "GlobalEventGroup.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
void testHMIW(void * ptr);
EventGroupHandle_t xEventGroup;

void testHMIW(void * ptr)
{
	const TickType_t errDelay = pdMS_TO_TICKS( portMAX_DELAY );
	HMIW_EV_INFO HMIWev;
 	for( ;; )
 	{
 		EventBits_t ev = xEventGroupWaitBits( xEventGroup,GEG_HMI,pdTRUE,pdFALSE,portMAX_DELAY);
 		if(ev & GEG_HMI)
 		{
 			HMIWev=HMIW_GetEvInfo();
 			if(HMIWev.id==INPUT_BUT_GREEN &&HMIWev.pat==LONG_PRESS)
 				HMIW_Blink(OUTPUT_LEDSTRIP_STOP,5);
 			HMIW_ListenToLongPress(INPUT_BUT_GREEN);
 		}

 	}
}



int main( void )
{
	HMIW_Init();
	HMIW_ListenToLongPress(INPUT_BUT_GREEN);
	xEventGroup =  xEventGroupCreate();
	BaseType_t ret = xTaskCreate(testHMIW, "CCO Test", 150	, NULL, 1, NULL );
	vTaskStartScheduler();
}
