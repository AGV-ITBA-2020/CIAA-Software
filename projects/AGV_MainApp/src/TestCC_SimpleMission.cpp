/*
 * TestCCO_SimpleMission.cpp
 *
 *  Created on: Nov 1, 2020
 *      Author: Javier
 */




#include "config.h"        // <= Biblioteca sAPI


#include "ControlCenter.hpp"
#include "GlobalEventGroup.h"
#include "AgvDiagnostics.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

extern EventGroupHandle_t xEventGroup;

void vApplicaationMallocFailedHook(void)
 {
	 while(1)
		 printf("MALLOC_FAILED");
 }

 void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char *pcTaskName )
 {
	while(1)
		printf("Stack Overflow! \n");

 }


int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	xEventGroup =  xEventGroupCreate();
	CC_init();
	vTaskStartScheduler();
   	return 0;
}
