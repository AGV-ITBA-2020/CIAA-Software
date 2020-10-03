/*
 * testComCenter.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: Javier
 */

#include "CommunicationCenter.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "my_sapi.h"
#include "event_groups.h"
/*
EventGroupHandle_t xEventGroup;
MISSION_T mission;

void testComCenter(void * ptr);

void testComCenter(void * ptr)
{

	for( ;; )
	{
		xEventGroupWaitBits( xEventGroup,1,pdFALSE,pdFALSE,portMAX_DELAY);
		MSG_REC_HEADER_T type = CCO_getMsgType();
		if(type==CCO_NEW_MISSION)
			CCO_getMission(&mission);
	}

}

int main(void)
{
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	xEventGroup =  xEventGroupCreate();
	CCO_init(xEventGroup);
	BaseType_t ret = xTaskCreate(testComCenter, "CCO Test", 512	, NULL, 1, NULL ); //Task para debuggear lo enviado
	if(ret==pdPASS)
		vTaskStartScheduler();
}

*/
