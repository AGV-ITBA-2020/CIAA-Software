/*
 * SimpleForkTest.cpp
 *
 *  Created on: Nov 21, 2020
 *      Author: Javier
 */




#include "config.h"        // <= Biblioteca sAPI

#include "ControlCenter.hpp"
#include "PathControlProcess.h"
#include "MovementControlModule.hpp"
#include "GlobalEventGroup.h"
#include "AgvDiagnostics.hpp"
#include "SecuritySystem.hpp"
#include "CommunicationCenter.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

// extern EventGroupHandle_t xEventGroup;


int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	AgvDiag_Init();
    CC_Init();
	#ifdef DEBUG_WITHOUT_MC
		MC_Init();
	#endif
	vTaskStartScheduler();
   	return 0;
}

void vApplicaationMallocFailedHook(void)
{
	while(1)
		printf("MALLOC_FAILED");
}

void vApplicationDaemonTaskStartupHook(void)
{
	while(1)
		printf("STARTUP_FAILED");
}

void vApplicationIdleHook( void )
{
	while(1)
		printf("IDLE_FAILED");
}

 void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char *pcTaskName )
 {
	while(1)
		printf("Stack Overflow! \n");
 }
