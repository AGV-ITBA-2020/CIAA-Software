/*
 * SimpleForkTest.cpp
 *
 *  Created on: Nov 21, 2020
 *      Author: Javier
 */

/*


#include "config.h"        // <= Biblioteca sAPI


#include "PathControlProcess.h"
#include "MovementControlModule.hpp"
#include "GlobalEventGroup.h"
#include "AgvDiagnostics.hpp"
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
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,GEG_COMS_RX,pdTRUE,pdFALSE,portMAX_DELAY); // @suppress("Invalid arguments")
		if(ev & GEG_COMS_RX)
		{
			int debug =1;
			MSG_REC_HEADER_T type = CCO_getMsgType();
			if(type==CCO_SET_VEL)
				PCP_SetLinearSpeed(CCO_getLinSpeed());
			else
				PCP_SetLinearSpeed(0);
		}
		else
			PCP_SetLinearSpeed(0);
		vTaskDelay(xDelay50ms);

	}

}


int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	blockTest.blockCheckpoints[0]=CHECKPOINT_FORK_LEFT;
	blockTest.blockCheckpoints[1]=CHECKPOINT_MERGE;
	blockTest.blockCheckpoints[2]=CHECKPOINT_FORK_RIGHT;
	blockTest.blockCheckpoints[3]=CHECKPOINT_MERGE;
	blockTest.blockCheckpoints[4]=CHECKPOINT_STATION;
	blockTest.currStep=0;
	blockTest.blockLen=5;
	xEventGroup =  xEventGroupCreate();
	CCO_init();
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
 }*/
