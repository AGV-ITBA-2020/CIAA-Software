/*
 * TestPCP.cpp
 *
 *  Created on: Oct 28, 2020
 *      Author: Javier
 */

#include "config.h"        // <= Biblioteca sAPI


#include "PathControlProcess.h"
#include "GlobalEventGroup.h"
#include "AgvDiagnostics.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
/*
extern EventGroupHandle_t xEventGroup;

static BLOCK_DETAILS_T blockTest;
int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	blockTest.blockCheckpoints[0]=CHECKPOINT_STATION;
	blockTest.currStep=0;
	blockTest.blockLen=1;
	xEventGroup =  xEventGroupCreate();
	AgvDiag_Init();
	PCP_Init();
	PCP_startNewMissionBlock(&blockTest);
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

*/

