/*
 * PathControlModule.c
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */


/*==================[inclusions]=============================================*/
#include "my_sapi_uart.h"

#include "PathControlModule.h"
#include "PathControlProcess.h"
#include "MovementControlModule.hpp"
#include "GlobalEventGroup.h"

#include "event_groups.h"
#include "semphr.h"


/*==================[macros and definitions]=================================*/

typedef enum{PC_IDLE,PC_RUN,PC_PAUSE, PC_ERROR} PC_State;



/*==================[internal data declaration]==============================*/
extern EventGroupHandle_t xEventGroup;
static PC_State state, prevState;
static BLOCK_DETAILS_T * missionBlock;
static SemaphoreHandle_t xBinarySemaphore;
// static TaskHandle_t * missionTask;

/*==================[internal functions declaration]=========================*/
void pcmMainTask(void * ptr);
void mainFSM(EventBits_t ev);
void idleParseEv(EventBits_t ev);
void runParseEv(EventBits_t ev);
void pauseParseEv(EventBits_t ev);
void errorParseEv(EventBits_t ev);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/*******Tasks*********/
void pcmMainTask(void * ptr)
{
	const TickType_t timeoutDelay250ms = pdMS_TO_TICKS( 250 );
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,PC_EVENT_MASK,pdTRUE,pdFALSE,timeoutDelay250ms );
		mainFSM(ev); //Con el evento que llega se ejecuta la máquina de estados
	}
}

/*******Otros*********/
void mainFSM(EventBits_t ev)
{
	switch(state){
		case PC_IDLE: //Funciones que dividen la FSM dependiendo del estado en el que se est�
			idleParseEv(ev);
			break;
		case PC_RUN:
			runParseEv(ev);
			break;
		case PC_PAUSE:
			pauseParseEv(ev);
			break;
		case PC_ERROR:
			errorParseEv(ev);
			break;
		default:
			break;
	}
}

void idleParseEv(EventBits_t ev)
{
	if(ev & GEG_MISSION_BLOCK_STARTED)
	{
		PCP_startNewMissionBlock(missionBlock);
		state=PC_RUN;
	}
}

void runParseEv(EventBits_t ev)
{
	if(ev & GEG_MISSION_ABORT_CMD)
	{
		PCP_abortMissionBlock();
		state=PC_IDLE;
	}
	else if(ev & GEG_MISSION_PAUSE_CMD)
	{
		PCP_pauseMissionBlock();
		state=PC_PAUSE;
	}
}

void pauseParseEv(EventBits_t ev)
{
	if(ev & GEG_CONTINUE)
	{
		PCP_continueMissionBlock();
		state=PC_RUN;
	}
}

void errorParseEv(EventBits_t ev)
{
}

/*==================[external functions definition]==========================*/
void PC_Init(void)
{
	MC_Init();
	PCP_Init();
	xTaskCreate( pcmMainTask, "PC Main task", 100	, NULL, 1, NULL ); //Crea task de misi�n
}

void PC_setMissionBlock(BLOCK_DETAILS_T * mb)
{
	missionBlock = mb;
	xEventGroupSetBits( xEventGroup, GEG_MISSION_BLOCK_STARTED );
}
