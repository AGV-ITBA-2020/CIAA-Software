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
#define DIST_ESTIMATOR_SAMPLE_PERIOD_MS 200



/*==================[internal data declaration]==============================*/
extern EventGroupHandle_t xEventGroup;
static PC_State state, prevState;
static BLOCK_DETAILS_T * missionBlock;
static SemaphoreHandle_t xBinarySemaphore;
static double distTravelled;
// static TaskHandle_t * missionTask;

/*==================[internal functions declaration]=========================*/
void pcmMainTask(void * ptr);
void mainFSM(EventBits_t ev);
void distEstFSM(EventBits_t ev);
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
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,PC_EVENT_MASK,pdTRUE,pdFALSE,portMAX_DELAY);	// portMAX_DELAY si no quieron timeout
		mainFSM(ev); //Con el evento que llega se ejecuta la mÃ¡quina de estados // @suppress("Invalid arguments")
		distEstFSM(ev);
	}
}
void pcmDistEstTask(void * ptr)
{
	const TickType_t timeoutDelay = pdMS_TO_TICKS( DIST_ESTIMATOR_SAMPLE_PERIOD_MS );
	TickType_t xLastTimeWoke = xTaskGetTickCount();
	for( ;; )
	{ //Podría fijarse que si la distTravelled da mayor a lo esperado en distancias a recorrer que se quede ahí (viendo la estructura de la misión)
		if(state==PC_RUN) //Solo actualiza la distancia cuando está corriendo el programa
			distTravelled += PCP_getLinearSpeed()* float(DIST_ESTIMATOR_SAMPLE_PERIOD_MS)/1000.0;// portMAX_DELAY si no quieron timeout
		vTaskDelayUntil(&xLastTimeWoke, timeoutDelay);
	}
}

/*******Otros*********/
void mainFSM(EventBits_t ev)
{
	switch(state){
		case PC_IDLE: //Funciones que dividen la FSM dependiendo del estado en el que se estï¿½
			idleParseEv(ev);	// @suppress("Invalid arguments")
			break;
		case PC_RUN:
			runParseEv(ev);		// @suppress("Invalid arguments")
			break;
		case PC_PAUSE:
			pauseParseEv(ev);	// @suppress("Invalid arguments")
			break;
		case PC_ERROR:
			errorParseEv(ev);	// @suppress("Invalid arguments")
			break;
		default:
			break;
	}
}

void distEstFSM(EventBits_t ev)
{
	if(ev & ( GEG_MISSION_STEP_REACHED) ) //Cuando llega a un punto de la mision se resetea el estimador de distancia.
		distTravelled=0;
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
	else if(ev & GEG_CTMOVE_FINISH)
	{
		state=PC_IDLE;
	}
}

void pauseParseEv(EventBits_t ev)
{
	if(ev & GEG_CONTINUE)
	{
		PCP_continueMissionBlock();
		state=PC_RUN;
	}
	if(ev & GEG_MISSION_ABORT_CMD)
	{
		PCP_abortMissionBlock();
		state=PC_IDLE;
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
	xTaskCreate( pcmMainTask, "PC Main task", 100	, NULL, 1, NULL ); //Crea task de misiï¿½n
	xTaskCreate( pcmDistEstTask, "PCM dist estimator", 100	, NULL, 1, NULL ); //Task para estimación de la dist recorrida
}

void PC_setMissionBlock(BLOCK_DETAILS_T * mb)
{
	distTravelled = 0;
	missionBlock = mb;
	xEventGroupSetBits( xEventGroup, GEG_MISSION_BLOCK_STARTED );
}
double PC_getDistTravelled()
{
	return distTravelled;
}
