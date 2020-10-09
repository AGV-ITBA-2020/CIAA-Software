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
static CC_State state;
static MISSION_T currMission;

void CC_mainTask(void *);
void CC_mainFSM(EventBits_t ev);
void CC_onMissionParseEv(EventBits_t ev);
MISSION_BLOCK_T getNextMissionBlock();
bool_t isMissionCompleted();
void missionAdvance();
/*==================[internal functions definition]==========================*/
void CC_mainTask(void *)
{
	const TickType_t errDelay = pdMS_TO_TICKS( 12000 );
	while(!CCO_connected());
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,1,pdTRUE,pdFALSE,errDelay);

		if(state==CC_ON_MISSION)
			CC_onMissionParseEv(ev);
		//Y así con distintas funciones parser para cada estado
		CC_mainFSM(ev);


	}
}
void CC_mainFSM(EventBits_t ev)
{
	MSG_REC_HEADER_T recHeader;
	if(ev & GEG_COMS_RX)
		recHeader=CCO_getMsgType(); //Si fue un mensaje de internet, recibe el header
	if((recHeader == CCO_NEW_MISSION) && state==CC_IDLE) //Si llegó una nueva misión estando en IDLE
	{
		state=CC_ON_MISSION; //Pasa a estado misión
		CCO_getMission(&currMission);
		CCO_sendMsgWithoutData(CCO_MISSION_ACCEPT); //Le comunica a houston que acepta la misión
		if(!currMission.waitForInterBlockEvent) //En el caso que no necesite un evento extra para arrancar la misión
			PC_setMissionBlock(getNextMissionBlock());
	}
	if((ev & GEG_MISSION_STEP_REACHED) && isMissionCompleted())
		state=CC_IDLE;
}
void CC_onMissionParseEv(EventBits_t ev)
{
	if(ev & GEG_MISSION_STEP_REACHED) //Se llegó a un step de misión
	{
		CCO_sendMsgWithoutData(CCO_MISSION_STEP_REACHED);
		missionAdvance();
		if(!isMissionCompleted() && !currMission.waitForInterBlockEvent)
			PC_setMissionBlock(getNextMissionBlock());
	}
}
void missionAdvance() //Estas funciones de acá para abajo le darían motivo a una clase misión
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
/*==================[external functions declaration]=========================*/
void CC_init()
{
	PC_Init();
	CCO_init();
	xEventGroup =  xEventGroupCreate();
	xTaskCreate(CC_mainTask, "CC Main task", 100, NULL, 1, NULL ); //Crea task de misión
	state=CC_IDLE;
	currMission.active=0;
}
