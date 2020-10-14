/*
 * ControlCenter.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: Javier
 */

#include "CommunicationCenter.hpp"
#include "PathControl.h"
#include "GlobalEventGroup.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"


/*==================[defines]================================================*/


/*==================[typedef]================================================*/

typedef enum{CC_IDLE,CC_ON_MISSION,CC_MANUAL, CC_ERROR, CC_PAUSE, CC_LOWPOWER} CC_State;

/*==================[internal data declaration]==============================*/
extern EventGroupHandle_t xEventGroup;
static CC_State state;
static MISSION_T currMission;

void CC_mainTask(void *);
void CC_mainFSM(EventBits_t ev);

void CC_onMissionParseEv(EventBits_t ev);
void CC_idleParseEv(EventBits_t ev);
BLOCK_DETAILS_T * getNextMissionBlock();
bool_t isMissionCompleted();
void missionAdvance();
/*==================[internal functions definition]==========================*/
void CC_mainTask(void *)
{
	const TickType_t errDelay = pdMS_TO_TICKS( 12000 );
	while(!CCO_connected());
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,CC_EVENT_MASK,pdTRUE,pdFALSE,errDelay);
		CC_mainFSM(ev);


	}
}
void CC_mainFSM(EventBits_t ev)
{
	

	switch(state){
		case CC_IDLE:
			CC_idleParseEv(ev);
			break;
		case CC_ON_MISSION:
			CC_onMissionParseEv(ev);
			break;
		case CC_ERROR:
			//CC_idleParseEv(ev);
			break;
		case CC_MANUAL:
			//CC_idleParseEv(ev);
			break;
		default:
			break;
	}
}
void CC_idleParseEv(EventBits_t ev)
{
	MSG_REC_HEADER_T recHeader;
	if(ev & GEG_COMS_RX){
		recHeader=CCO_getMsgType(); //Si fue un mensaje de internet, recibe el header
	}
	if((ev & GEG_COMS_RX) && recHeader == CCO_NEW_MISSION)
	{
		state=CC_ON_MISSION; //Pasa a estado misi�n
		CCO_getMission(&currMission);
		CCO_sendMsgWithoutData(CCO_MISSION_ACCEPT); //Le comunica a houston que acepta la misi�n
		//if(!currMission.waitForInterBlockEvent) //En el caso que no necesite un evento extra para arrancar la misi�n
			//PC_setMissionBlock(getNextMissionBlock());
	}
}
void CC_onMissionParseEv(EventBits_t ev)
{
	if(ev & GEG_MISSION_STEP_REACHED) //Se lleg� a un step de misi�n
	{
		CCO_sendMsgWithoutData(CCO_MISSION_STEP_REACHED);
		missionAdvance();
		//if(!isMissionCompleted() && !currMission.waitForInterBlockEvent)
			//PC_setMissionBlock(getNextMissionBlock());
	}
	else if(ev & GEG_CTMOVE_FINISH) //Se lleg� a un step de misi�n
	{
		CCO_sendMsgWithoutData(CCO_MISSION_STEP_REACHED);
		missionAdvance();
		if(!isMissionCompleted())
			state=CC_IDLE;
	}
}
void missionAdvance() //Estas funciones de ac� para abajo le dar�an motivo a una clase misi�n
{
	currMission.currBlock++;
	if(currMission.interBlockEvent[currMission.currBlock]==IBE_NONE)
		currMission.waitForInterBlockEvent=false;
	else
		currMission.waitForInterBlockEvent=true;
}
bool_t isMissionCompleted()
{
	return (currMission.currBlock==currMission.nmbrOfBlocks) && (!currMission.waitForInterBlockEvent);
}
BLOCK_DETAILS_T * getNextMissionBlock()
{
	return &(currMission.blocks[currMission.currBlock]);
}
/*==================[external functions declaration]=========================*/
void CC_init()
{
	PC_Init();
	CCO_init();
	xEventGroup =  xEventGroupCreate();
	xTaskCreate(CC_mainTask, "CC Main task", 100, NULL, 1, NULL ); //Crea task de misi�n
	state=CC_IDLE;
	currMission.active=0;
}
