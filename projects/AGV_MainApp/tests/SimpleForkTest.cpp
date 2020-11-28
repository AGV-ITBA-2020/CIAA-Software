/*
 * SimpleForkTest.cpp
 *
 *  Created on: Nov 21, 2020
 *      Author: Javier
 */


/*

#include "config.h"        // <= Biblioteca sAPI

#include "HMIWrapper.hpp"
#include "PathControlProcess.h"
#include "MovementControlModule.hpp"
#include "GlobalEventGroup.h"
#include "AgvDiagnostics.hpp"
#include "SecuritySystem.hpp"
#include <CommunicationCenter.hpp>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

extern EventGroupHandle_t xEventGroup;

static BLOCK_DETAILS_T blockTest;

void testComCenter(void * ptr);

void testComCenter(void * ptr)
{
	const TickType_t errDelay = pdMS_TO_TICKS( 12000 );
	const TickType_t xDelay50ms = pdMS_TO_TICKS( 50 );
	while(!CCO_connected());
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,GEG_COMS_RX | GEG_MISSION_STEP_REACHED | GEG_EMERGENCY_STOP,pdTRUE,pdFALSE,portMAX_DELAY); // @suppress("Invalid arguments")
		if(ev & GEG_MISSION_STEP_REACHED)
		{
			static bool_t debug = 0;
//			HMIW_SetOutput(OUTPUT_LEDSTRIP_LEFT, !debug); // @suppress("Invalid arguments")
			HMIW_Blink(OUTPUT_LEDSTRIP_RIGHT,5);
			HMIW_Blink(OUTPUT_LEDSTRIP_LEFT,5);
		}
		else if (ev & GEG_EMERGENCY_STOP)
		{
			PCP_SetLinearSpeed(0);
		}
		vTaskDelay(xDelay50ms);

	}

}


int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	blockTest.blockCheckpoints[0]=CHECKPOINT_STATION;
	blockTest.blockCheckpoints[1]=CHECKPOINT_FORK_RIGHT;
	blockTest.blockCheckpoints[2]=CHECKPOINT_MERGE;
	blockTest.blockCheckpoints[3]=CHECKPOINT_FORK_LEFT;
	blockTest.blockCheckpoints[4]=CHECKPOINT_MERGE;
	blockTest.currStep=0;
	blockTest.blockLen=5;
	xEventGroup =  xEventGroupCreate();
	HMIW_Init();
	HMIW_ListenToShortPress(INPUT_BUT_GREEN);
	HMIW_ListenToShortPress(INPUT_BUT_BLUE);
	SS_init();
	AgvDiag_Init();
	CCO_init();
	#ifdef DEBUG_WITHOUT_MC
		MC_Init();
	#endif
	PCP_Init();
	PCP_startNewMissionBlock(&blockTest);
	BaseType_t ret = xTaskCreate(testComCenter, "CCO Test", 150	, NULL, 1, NULL ); //Task para debuggear lo enviado
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