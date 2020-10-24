#include "config.h"        // <= Biblioteca sAPI
#include <ControlCenter.hpp>
#include <CommunicationCenter.hpp>
#include <PathControlModule.h>
#include "MovementControlModule.hpp"
#include "GlobalEventGroup.h"

#include "AgvDiagnostics.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "assert.h"




EventGroupHandle_t xEventGroup;
MISSION_T mission;

 void testComCenter(void * ptr);

 void testComCenter(void * ptr)
 {
 	const TickType_t errDelay = pdMS_TO_TICKS( 12000 );
	const TickType_t xDelay50ms = pdMS_TO_TICKS( 50 );
 //	while(!CCO_connected());
 	for( ;; )
 	{
// 		EventBits_t ev = xEventGroupWaitBits( xEventGroup,1,pdTRUE,pdFALSE,errDelay);
// 		if(ev & GEG_COMS_RX)
// 		{
// 			int debug =1;
// 			//MSG_REC_HEADER_T type = CCO_getMsgType();
// //			if(type==CCO_SET_VEL)
// //			{
// 				MC_setLinearSpeed(CCO_getLinSpeed());
// 				MC_setAngularSpeed(CCO_getAngSpeed());
// //			}
// //			else
// //				assert(0);
// 		}
// 		else
// 			assert(0);
		vTaskDelay(xDelay50ms);

 	}

 }

int main( void )
{

	// Read clock settings and update SystemCoreClock variable
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);

	bool_t initOk = true;

	// CC_init();

	// xEventGroup =  xEventGroupCreate();
	// CCO_init(xEventGroup);
	// CCO_init();
	AgvDiag_Init();

	// initOk = PC_Init();
	MC_Init();

//	 BaseType_t ret = xTaskCreate(testComCenter, "CCO Test", 100	, NULL, 1, NULL ); //Task para debuggear lo enviado
//	printf( "Starting RTOS...\r\n" );
	vTaskStartScheduler();

	// if(initOk == true)
	// {
	// 	printf( "Starting RTOS...\r\n" );
	// 	vTaskStartScheduler();
	// }

   	return 0;
}



