/*
 * ControlCenter.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: Javier
 */

#include "CommunicationCenter.hpp"
#include "PathControlModule.h"
#include "HMIWrapper.hpp"
#include "GlobalEventGroup.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"


/*==================[defines]================================================*/


/*==================[typedef]================================================*/

typedef enum{CC_IDLE,CC_ON_MISSION,CC_MANUAL, CC_ERROR, CC_PAUSE, CC_EMERGENCY,CC_LOWPOWER} CC_State;
#define N_PRESSES_ON_EMERGENCY 2
#define N_PRESSES_TO_ABORT_MISSION 3
/*==================[internal data declaration]==============================*/
extern EventGroupHandle_t xEventGroup;
static CC_State state,prevState;
static MISSION_T currMission;
static MSG_REC_HEADER_T recHeader;
static AGV_STATUS_T agvStatus;
static HMIW_EV_INFO hmiEv;

void CC_mainTask(void *);
void CC_notifyStatus(void *);
void CC_mainFSM(EventBits_t ev);

void CC_onErrorRoutine(EventBits_t ev);
void CC_indepParseEv(EventBits_t ev);
void CC_onMissionParseEv(EventBits_t ev);
void CC_idleParseEv(EventBits_t ev);
void CC_pauseParseEv(EventBits_t ev);
void CC_emergencyParseEv(EventBits_t ev);

void checkIfManualModeEnabled(EventBits_t ev);
void changeStateTo(CC_State newState);


INTER_BLOCK_EVENT_T currMissionIBE();
BLOCK_DETAILS_T * getNextMissionBlock();
bool_t isMissionCompleted();
void missionAdvance();
/*==================[internal functions definition]==========================*/
/*===============[Useful funcs]=====================*/
bool_t hmiEvCondition(EventBits_t ev,HMI_INPUT_ID id, HMI_INPUT_PATTERN pat)
{
	return ((ev & GEG_HMI) && hmiEv.id==id && hmiEv.pat==pat);
}
/*===============[Tasks]=====================*/
void CC_notifyStatus(void *)
{
	char * msgP;
	const TickType_t delay = pdMS_TO_TICKS( 2000 );
	for( ;; )
	{
		//getStatus();
		//CCO_sendStatus(agvStatus);
	}

}
void CC_mainTask(void *)
{
	const TickType_t timeoutDelay = pdMS_TO_TICKS( 12000 );
	while(!CCO_connected());
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,CC_EVENT_MASK,pdTRUE,pdFALSE,portMAX_DELAY );
		CC_mainFSM(ev); //Con el evento que llega se ejecuta la máquina de estados // @suppress("Invalid arguments")
	}
}
void CC_mainFSM(EventBits_t ev)
{
	if(ev & GEG_COMS_RX){
		recHeader=CCO_getMsgType(); //Si fue un mensaje de internet, recibe el header
	}
	if(ev & GEG_HMI){
		hmiEv=HMIW_GetEvInfo(); //Si fue un evento de HMI, se debería obtener que tipo de evento fue
	}
	CC_indepParseEv(ev); //Eventos que no dependen de su estado inicial (van a error)
	switch(state){
		case CC_IDLE: //Funciones que dividen la FSM dependiendo del estado en el que se esté
			CC_idleParseEv(ev);
			break;
		case CC_ON_MISSION:
			CC_onMissionParseEv(ev);
			break;
		case CC_PAUSE:
			CC_pauseParseEv(ev);
			break;
		case CC_EMERGENCY:
			CC_emergencyParseEv(ev);
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
void CC_indepParseEv(EventBits_t ev)
{
	if(ev == 0) //Si hubo un timeout. En principio no pasaría nada no?
	{}
	if(ev & ERROR_EVENT_MASK)
	{
		changeStateTo(CC_ERROR);
		CC_onErrorRoutine(ev);
	}
	if(ev & GEG_EMERGENCY_STOP) //Se llego a un step de mision
		CCO_sendMsgWithoutData(CCO_EMERGENCY_STOP);
	if(ev & GEG_PRIORITY_STOP) //Se llego a un step de mision
		CCO_sendMsgWithoutData(CCO_PRIORITY_STOP);
}
void CC_idleParseEv(EventBits_t ev)
{
	if((ev & GEG_COMS_RX) && recHeader == CCO_NEW_MISSION)
	{
		CCO_getMission(&currMission);
		CCO_sendMsgWithoutData(CCO_MISSION_ACCEPT); //Le comunica a houston que acepta la mision
		PC_setMissionBlock(getNextMissionBlock());
		if(!currMission.waitForInterBlockEvent) //En el caso que no necesite un evento extra para arrancar la misiï¿½n
		{
			changeStateTo(CC_ON_MISSION); //Pasa a estado misiï¿½n
			xEventGroupSetBitsFromISR( xEventGroup, GEG_MISSION_BLOCK_STARTED, NULL );
		}
		else
			changeStateTo(CC_PAUSE); //Sino espera hasta que ocurra ese evento
	}
	checkIfManualModeEnabled(ev);
}
void CC_onMissionParseEv(EventBits_t ev)
{
	if(ev & GEG_MISSION_STEP_REACHED) //Se llego a un step de mision
	{
		CCO_sendMsgWithoutData(CCO_MISSION_STEP_REACHED);
		missionAdvance();
	}
	if(ev & GEG_CTMOVE_FINISH) //Se termino el bloque que se le habia mandado al PC
	{
		CCO_sendMsgWithoutData(CCO_MISSION_STEP_REACHED);
		missionAdvance();
		if(currMission.waitForInterBlockEvent) //Si se tiene que esperar por un evento se va a pausa
			changeStateTo(CC_PAUSE);
		else if(!isMissionCompleted()) //Sino, si todavía no terminó la misión le paso el bloque que viene
		{
			PC_setMissionBlock(getNextMissionBlock());
			xEventGroupSetBits( xEventGroup, GEG_MISSION_BLOCK_STARTED);
		}
		else //En caso de que ya la haya terminado, voy a IDLE
			changeStateTo(CC_IDLE);
	}
	if(ev & (GEG_EMERGENCY_STOP | GEG_PRIORITY_STOP )) //Cualquier emergencia va a estado de emergencia
		changeStateTo(CC_PAUSE);
	if((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) //Recibe que aborte misión por internes
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		changeStateTo(CC_IDLE);
	}

	if((ev & GEG_COMS_RX) && recHeader == CCO_PAUSE_MISSION)//Recibe que pause misión por internes
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_PAUSE_CMD);
		changeStateTo(CC_PAUSE);
	}

	checkIfManualModeEnabled(ev);
}

void CC_pauseParseEv(EventBits_t ev)
{
	/*Esto sería correspondiente a pausa */
	if(currMission.waitForInterBlockEvent) //En el caso en que se este esperando por un evento entre bloques.
	{
		if(((currMissionIBE()== IBE_HOUSTON_CONTINUE) && (ev & GEG_COMS_RX) && (recHeader == CCO_CONTINUE)) || //Si se esperaba un continue de houston y llegó
		   ((currMissionIBE()== IBE_BUTTON_PRESS) && hmiEvCondition(ev,INPUT_BUT_BLUE, LONG_PRESS))){ //o si se esperaba presionar un botón y se presionó
			currMission.waitForInterBlockEvent=false; //Ya no se tiene que esperar por evento.
			CCO_sendMsgWithoutData(CCO_IBE_RECIEVED); //Comunica a Houston que recibió el IBE
			if(!isMissionCompleted()) //Si faltan bloques de la misión
			{
				PC_setMissionBlock(getNextMissionBlock());
				xEventGroupSetBits( xEventGroup, GEG_MISSION_BLOCK_STARTED);
				changeStateTo(CC_ON_MISSION);
			}
			else //Si este evento era el último paso de la misión, vuelve a IDLE
				changeStateTo(CC_IDLE);
		}
	}
	if((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) //Si estando en pausa recibe que aborte misión.
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		changeStateTo(CC_IDLE);
	}
	checkIfManualModeEnabled(ev);
}
void CC_emergencyParseEv(EventBits_t ev)
{
	if(((ev & GEG_COMS_RX) && recHeader == CCO_CONTINUE) || hmiEvCondition(ev,INPUT_BUT_GREEN, COUNTER))
	{
//		if(chekcOnEmergency()) //Se fija que no siga estando en emergencia
//		{
			changeStateTo(prevState);
			//xEventGroupSetBits( xEventGroup, GEG_MISSION_CONTINUE_CMD); //Si vuelve a estado misión debería poner esto
//		}
	}
}
void CC_onErrorRoutine(EventBits_t ev)
{
	//Pasar por todos los módulos con eventos de error y recupera el string de error
	//Lo comunica a houston salvo que sea algo del centro de comunicaciones (CCO_sendError(string err))
}


void checkIfManualModeEnabled(EventBits_t ev)
{
//		if((ev & GEG_HMI) && HMIevType==MANUAL_MODE){
//			changeStateTo(CC_MANUAL);
//			if(state==CC_ON_MISSION)
//				xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
//		}
}
void changeStateTo(CC_State newState)
{
	prevState=state;
	state=newState;
	if(state==CC_EMERGENCY)
	{
		HMIW_ListenToMultiplePress(INPUT_BUT_GREEN, N_PRESSES_ON_EMERGENCY);
		HMIW_Blink(OUTPUT_LEDSTRIP_STOP, 5);
	}
	else if(state==CC_ON_MISSION)
	{
		//HMIW_ListenToMultiplePress(INPUT_BUT_BLUE, N_PRESSES_TO_ABORT_MISSION);
		HMIW_Blink(OUTPUT_BUT_GREEN, 5);
	}
	else if(state==CC_PAUSE)
	{
		HMIW_ListenToLongPress(INPUT_BUT_BLUE);
		HMIW_Blink(OUTPUT_BUT_BLUE, 5);
	}

}

/*Estas funciones de abajo le darían motivo a una clase misión (con funciones propias) */
void missionAdvance()
{
	currMission.currBlock++; //Avanza el bloque
	if(currMission.interBlockEvent[currMission.currBlock]==IBE_NONE) //Se fija si necesita un evento para avanzar al siguiente bloque
		currMission.waitForInterBlockEvent=false;
	else
		currMission.waitForInterBlockEvent=true;
}
bool_t isMissionCompleted() //Termina si ya pasó todos los bloques y no espera ningún evento
{
	return (currMission.currBlock==currMission.nmbrOfBlocks) && (!currMission.waitForInterBlockEvent);
}
INTER_BLOCK_EVENT_T currMissionIBE() //Devuelvo el Evento esperado para avanzar
{
	return currMission.interBlockEvent[currMission.currBlock];
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
	xTaskCreate(CC_mainTask, "CC Main task", 100, NULL, 1, NULL ); //Crea task de misiï¿½n
	state=CC_IDLE;
	prevState=CC_IDLE;
	currMission.active=0;
	//xTaskCreate( CC_notifyStatus, "CC notify status task", 100	, NULL, 1, NULL );
}
