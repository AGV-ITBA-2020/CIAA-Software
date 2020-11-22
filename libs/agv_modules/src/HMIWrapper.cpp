/*
 * HMIWrapper.cpp
 *
 *  Created on: Nov 22, 2020
 *      Author: Javier
 */

#include "HMIWrapper.hpp"
#include "GlobalEventGroup.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"


#define HMIW_EV_QUEUE_LEN 5
extern EventGroupHandle_t xEventGroup;
static QueueHandle_t evQueue;
/*==================[internal functions declaration]==========================*/
void inputCallback(HMI_INPUT_ID id);
static HMI_INPUT_PATTERN patternConfig[INPUT_TOTAL_COUNT];
void dumbFunc(HMI_OUTPUT_ID id);
/*==================[external functions definition]=========================*/


void HMIW_Init()
{
	HMI_Init();
	evQueue=xQueueCreate( HMIW_EV_QUEUE_LEN,sizeof(HMIW_EV_INFO));
}
void HMIW_ListenToLongPress(HMI_INPUT_ID id)
{
	HMI_Input_t exampleInput;
	exampleInput.IO_type=HMI_INPUT;
	exampleInput.id=id;
	exampleInput.pattern=LONG_PRESS; //Creo que esto ni hace falta ponerlo.
	exampleInput.patCount=1;
	exampleInput.count=0;
	exampleInput.inputPin=HMI_getCorrespondingPin(exampleInput.IO_type, exampleInput.id);
	exampleInput.maxCount=int(HMIW_LONGPRESS_MS/HMI_REFRESH_MS);
	exampleInput.callbackSuccess=inputCallback;
	HMI_AddToQueue((void *)&exampleInput);
	patternConfig[id]=LONG_PRESS;
}
void HMIW_ListenToMultiplePress(HMI_INPUT_ID id, unsigned int count)
{
	HMI_Input_t exampleInput;
	exampleInput.IO_type=HMI_INPUT;
	exampleInput.id=id;
	exampleInput.pattern=COUNTER; //Creo que esto ni hace falta ponerlo.
	exampleInput.patCount=count;
	exampleInput.count=0;
	exampleInput.inputPin=HMI_getCorrespondingPin(exampleInput.IO_type, exampleInput.id);
	exampleInput.maxCount=int(HMIW_MIN_PRESS_MS/HMI_REFRESH_MS);
	exampleInput.callbackSuccess=inputCallback;
	HMI_AddToQueue((void *)&exampleInput);
	patternConfig[id]=LONG_PRESS;
}
void HMIW_Blink(HMI_OUTPUT_ID id, unsigned int count)
{
	HMI_Output_t tempObj;
	tempObj.IO_type=HMI_OUTPUT;
	tempObj.id=id;
	tempObj.outputPin=HMI_getCorrespondingPin(tempObj.IO_type, tempObj.id);
	tempObj.timebaseCounter=0;
	tempObj.timeOn=10;
	tempObj.timeOff=10;
	tempObj.actionCounter=count;
	tempObj.callbackSuccess=dumbFunc;
	HMI_AddToQueue((void *)&tempObj);
}

HMIW_EV_INFO HMIW_GetEvInfo()
{
	HMIW_EV_INFO retVal;
	BaseType_t bt= xQueueReceive( evQueue,&retVal,0); //Si hay algo en la cola lo pone en msgP
	if(uxQueueMessagesWaiting(evQueue))
		xEventGroupSetBits( xEventGroup,GEG_HMI );
	return retVal; //Esto dice si se leyó algo o no.
}
/*==================[ callbacks]==========================*/
void inputCallback(HMI_INPUT_ID id)
{
	HMIW_EV_INFO aux;
	aux.id=id;
	aux.pat=patternConfig[id];
	xQueueSendToBack(evQueue,&aux,0);
	xEventGroupSetBits( xEventGroup,GEG_HMI );
}
void dumbFunc(HMI_OUTPUT_ID id)
{

}
