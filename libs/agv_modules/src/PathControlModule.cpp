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
#define OPEN_MV_MSG_LEN 4 //Length en bytes del mensaje del openMV
#define MAX_DISPLACEMENT 64
#define EVENT_QUEUE_LEN 10

#define LOW_SPEED_VEL 0.5
#define HIGH_SPEED_VEL 1.0

typedef enum{PC_IDLE,PC_RUN,PC_PAUSE, PC_ERROR} PC_State;

typedef enum {OPENMV_FOLLOW_LINE, OPENMV_FORK_LEFT, OPENMV_FORK_RIGHT, OPENMV_MERGE, OPENMV_ERROR,OPENMV_IDLE}openMV_states; //Los distintos estados del OpenMV
typedef enum {TAG_SLOW_DOWN, TAG_SPEED_UP, TAG_STATION=3}Tag_t; //Los distintos TAGs

#define IS_FORKORMERGE_MISSION(x) ((x)==CHECKPOINT_FORK_LEFT || (x) == CHECKPOINT_FORK_RIGHT || (x) == CHECKPOINT_MERGE )

typedef struct  {
  int displacement;
  bool_t tag_found;
  bool_t form_passed;
  bool_t error;
  Tag_t tag;
}openMV_msg; //La informaci�n que trae cada mensaje del openMV.


/*==================[internal data declaration]==============================*/
extern EventGroupHandle_t xEventGroup;
static PC_State state, prevState;
static MISSION_BLOCK_T * missionBlock;
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
	}
}

void runParseEv(EventBits_t ev)
{
	if(ev & GEG_MISSION_ABORT_CMD)
	{
		PCP_abortMissionBlock();
	}
}

void pauseParseEv(EventBits_t ev)
{
}

void errorParseEv(EventBits_t ev)
{
}

/*==================[external functions definition]==========================*/
void PC_Init(void)
{
	uartInit( PC_UART, PC_UART_BAUDRATE, 1 );
	MC_Init();
	// xBinarySemaphore = xSemaphoreCreateBinary();
	xTaskCreate( pcmMainTask, "PC Main task", 100	, NULL, 1, NULL ); //Crea task de misi�n
}

void PC_setMissionBlock(MISSION_BLOCK_T * mb)
{
	missionBlock = mb;
	xEventGroupSetBits( xEventGroup, GEG_MISSION_BLOCK_STARTED );
}
