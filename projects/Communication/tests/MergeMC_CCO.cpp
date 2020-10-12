/*
 * MergeMC_CCO.cpp
 *
 *  Created on: Oct 3, 2020
 *      Author: Javier
 */


/*
 * testComCenter.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: Javier
 */

#include "CommunicationCenter.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "assert.h"
#include "my_sapi.h"
#include "event_groups.h"
#include "MovementControlModule.h"
/*
EventGroupHandle_t xEventGroup;
MISSION_T mission;

void testComCenter(void * ptr);

void testComCenter(void * ptr)
{
	const TickType_t errDelay = pdMS_TO_TICKS( 12000 );
	while(!CCO_connected());
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,1,pdTRUE,pdFALSE,errDelay);
		if(ev & EV_CCO_MSG_REC)
		{
			int debug =1;
			//MSG_REC_HEADER_T type = CCO_getMsgType();
//			if(type==CCO_SET_VEL)
//			{
				MC_setLinearSpeed(CCO_getLinSpeed());
				MC_setAngularSpeed(CCO_getAngSpeed());
//			}
//			else
//				assert(0);
		}
		else
			assert(0);

	}

}

int main(void)
{
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	xEventGroup =  xEventGroupCreate();
	CCO_init(xEventGroup);
	MC_Init();
	BaseType_t ret = xTaskCreate(testComCenter, "CCO Test", 100	, NULL, 1, NULL ); //Task para debuggear lo enviado
	if(ret==pdPASS)
		vTaskStartScheduler();
}
*/
