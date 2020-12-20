/*
 * ControlCenter.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: Javier
 */

#include "CommunicationCenter.hpp"
#include "PathControlModule.h"
#include "HMIWrapper.hpp"
#include "SecuritySystem.hpp"
#include "GlobalEventGroup.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"


/*==================[defines]================================================*/


/*==================[typedef]================================================*/

typedef enum{CC_IDLE,CC_ON_MISSION,CC_MANUAL, CC_ERROR, CC_PAUSE, CC_EMERGENCY,CC_LOWPOWER} CC_State;
#define N_PRESSES_ON_EMERGENCY 2
// #define N_PRESSES_TO_ABORT_MISSION 3
// #define N_PRESSES_TO_PAUSE_MISSION 1
#define NOTIFY_STATUS_PERIOD_MS 10000
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
void genAgvStatusStruct();
/*==================[internal functions definition]==========================*/
/*===============[Useful funcs]=====================*/
bool_t hmiEvCondition(EventBits_t ev,HMI_INPUT_ID id, HMI_INPUT_PATTERN pat)
{
	return ((ev & GEG_HMI) && hmiEv.id==id && hmiEv.pat==pat);
}
void genAgvStatusStruct()
{
	agvStatus.distEst=PC_getDistTravelled();
	agvStatus.inMision=currMission.active;
	agvStatus.waitForInterBlockEvent=currMission.waitForInterBlockEvent;
	agvStatus.batVoltage = SS_GetBatteryLevel();
}
/*===============[Tasks]=====================*/
void CC_notifyStatus(void *)
{
	const TickType_t timeoutDelay = pdMS_TO_TICKS( NOTIFY_STATUS_PERIOD_MS );
	TickType_t xLastTimeWoke = xTaskGetTickCount();
	for( ;; )
	{
		genAgvStatusStruct();
		CCO_sendStatus(agvStatus);
		vTaskDelayUntil(&xLastTimeWoke, timeoutDelay);
	}

}
void CC_mainTask(void *)
{
	const TickType_t timeoutDelay = pdMS_TO_TICKS( 5000 );
	while(!CCO_connected());
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,CC_EVENT_MASK,pdTRUE,pdFALSE,portMAX_DELAY);
		CC_mainFSM(ev); //Con el evento que llega se ejecuta la m�quina de estados // @suppress("Invalid arguments")
	}
}
void CC_mainFSM(EventBits_t ev)
{
	if(ev & GEG_COMS_RX){
		recHeader=CCO_getMsgType(); //Si fue un mensaje de internet, recibe el header
	}
	if(ev & GEG_HMI){
		hmiEv=HMIW_GetEvInfo(); //Si fue un evento de HMI, se deber�a obtener que tipo de evento fue
	}
	CC_indepParseEv(ev); //Eventos que no dependen de su estado inicial (van a error)
	switch(state){
		case CC_IDLE: //Funciones que dividen la FSM dependiendo del estado en el que se est�
			CC_idleParseEv(ev);			// @suppress("Invalid arguments")
			break;
		case CC_ON_MISSION:
			CC_onMissionParseEv(ev);	// @suppress("Invalid arguments")
			break;
		case CC_PAUSE:
			CC_pauseParseEv(ev);		// @suppress("Invalid arguments")
			break;
		case CC_EMERGENCY:
			CC_emergencyParseEv(ev);	// @suppress("Invalid arguments")
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
	if(ev == 0) //Si hubo un timeout. En principio no pasar�a nada no?
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
		if(!currMission.waitForInterBlockEvent) //En el caso que no necesite un evento extra para arrancar la misi�n
		{
			HMIW_Blink(OUTPUT_BUT_GREEN, 3);
			changeStateTo(CC_ON_MISSION); //Pasa a estado misi�n
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
		if(currMission.waitForInterBlockEvent) //Si se tiene que esperar por un evento se va a pausa
			changeStateTo(CC_PAUSE);
		else if(!isMissionCompleted()) //Sino, si todav�a no termin� la misi�n le paso el bloque que viene
		{
			PC_setMissionBlock(getNextMissionBlock());
			xEventGroupSetBits( xEventGroup, GEG_MISSION_BLOCK_STARTED);
		}
		else //En caso de que ya la haya terminado, voy a IDLE
			changeStateTo(CC_IDLE);
	}
	if(ev & (GEG_EMERGENCY_STOP | GEG_PRIORITY_STOP )) //Cualquier emergencia va a estado de emergencia
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_PAUSE_CMD);
		changeStateTo(CC_EMERGENCY);
	}
	if(((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) || hmiEvCondition(ev,INPUT_BUT_BLUE, SHORT_PRESS)) //Recibe que aborte misi�n por internes
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		changeStateTo(CC_IDLE);
	}

	if(((ev & GEG_COMS_RX) && recHeader == CCO_PAUSE_MISSION) || hmiEvCondition(ev,INPUT_BUT_GREEN, SHORT_PRESS))//Recibe que pause misi�n por internes
	{
		HMIW_Blink(OUTPUT_BUT_GREEN, 10000);
		xEventGroupSetBits( xEventGroup, GEG_MISSION_PAUSE_CMD);
		changeStateTo(CC_PAUSE);
	}

	checkIfManualModeEnabled(ev);
}

void CC_pauseParseEv(EventBits_t ev)
{
	/*Esto ser�a correspondiente a pausa */
	if(((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) || hmiEvCondition(ev,INPUT_BUT_BLUE, LONG_PRESS)) //Si estando en pausa recibe que aborte misi�n.
	{
		HMI_ClearOutputs();
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		changeStateTo(CC_IDLE);
	}
	if(currMission.waitForInterBlockEvent) //En el caso en que se este esperando por un evento entre bloques.
	{
		if(((currMissionIBE()== IBE_HOUSTON_CONTINUE) && (ev & GEG_COMS_RX) && (recHeader == CCO_CONTINUE)) || //Si se esperaba un continue de houston y lleg�
		   ((currMissionIBE()== IBE_BUTTON_PRESS) && hmiEvCondition(ev,INPUT_BUT_GREEN, SHORT_PRESS)))	//o si se esperaba presionar un bot�n y se presion�
		{
			HMI_ClearOutputs();
			currMission.waitForInterBlockEvent=false; //Ya no se tiene que esperar por evento.
			CCO_sendMsgWithoutData(CCO_IBE_RECIEVED); //Comunica a Houston que recibi� el IBE
			if(!isMissionCompleted()) //Si faltan bloques de la misi�n
			{
				PC_setMissionBlock(getNextMissionBlock());
				xEventGroupSetBits( xEventGroup, GEG_MISSION_BLOCK_STARTED);
				changeStateTo(CC_ON_MISSION);
			}
			else //Si este evento era el �ltimo paso de la misi�n, vuelve a IDLE
				changeStateTo(CC_IDLE);
		}
	}else if(((ev & GEG_COMS_RX) && recHeader == CCO_CONTINUE) || hmiEvCondition(ev,INPUT_BUT_GREEN, SHORT_PRESS)) //Si estando en pausa recibe que aborte misi�n.
	{
		HMI_ClearOutputs();
		xEventGroupSetBits( xEventGroup, GEG_CONTINUE);
		changeStateTo(CC_ON_MISSION);
	}
	checkIfManualModeEnabled(ev);
}
void CC_emergencyParseEv(EventBits_t ev)
{
	if(((ev & GEG_COMS_RX) && recHeader == CCO_CONTINUE) || hmiEvCondition(ev,INPUT_BUT_GREEN, LONG_PRESS))
	{
		if(!SS_emergencyState()) //Se fija que no siga estando en emergencia
		{
			HMI_ClearOutputs();
			changeStateTo(prevState);
			xEventGroupSetBits( xEventGroup, GEG_CONTINUE); //Si vuelve a estado misi�n deber�a poner esto
		}
		else if(hmiEvCondition(ev,INPUT_BUT_GREEN, LONG_PRESS)) //Si se presion� el bot�n pero no se estaba en emergencia
			HMIW_ListenToLongPress(INPUT_BUT_GREEN); //Se escucha de vuelta al input.
	}
}
void CC_onErrorRoutine(EventBits_t ev)
{
	//Pasar por todos los m�dulos con eventos de error y recupera el string de error
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
	HMI_clearInputs(); //Borra todos los listens previos.
	if(state==CC_EMERGENCY)
	{
		HMIW_ListenToLongPress(INPUT_BUT_GREEN);
		HMIW_Blink(OUTPUT_LEDSTRIP_STOP, 10000);
	}
	else if(state==CC_ON_MISSION)
	{
		HMIW_ListenToShortPress(INPUT_BUT_BLUE);	// BLUE for AVORT
		HMIW_ListenToShortPress(INPUT_BUT_GREEN);	// GREEN for PAUSE
	}
	else if(state==CC_PAUSE)
	{
		HMIW_ListenToLongPress(INPUT_BUT_BLUE);	// BLUE for AVORT
		HMIW_ListenToShortPress(INPUT_BUT_GREEN);
	}

}

/*Estas funciones de abajo le dar�an motivo a una clase misi�n (con funciones propias) */
void missionAdvance()
{
	currMission.currBlock++; //Avanza el bloque
	if(currMission.interBlockEvent[currMission.currBlock]==IBE_NONE) //Se fija si necesita un evento para avanzar al siguiente bloque
		currMission.waitForInterBlockEvent=false;
	else
	{
		if(currMission.interBlockEvent[currMission.currBlock] == IBE_BUTTON_PRESS)
			HMIW_Blink(OUTPUT_BUT_GREEN, 10000);

		if(currMission.interBlockEvent[currMission.currBlock] == IBE_HOUSTON_CONTINUE)
			HMIW_SetOutput(OUTPUT_BUT_GREEN, true);

		currMission.waitForInterBlockEvent=true;

	}
}
bool_t isMissionCompleted() //Termina si ya pas� todos los bloques y no espera ning�n evento
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
void CC_Init()
{
	PC_Init();
	HMIW_Init();
	HMIW_ListenToShortPress(INPUT_BUT_GREEN);
	HMIW_ListenToShortPress(INPUT_BUT_BLUE);
	CCO_init();
	SS_init();
	xEventGroup =  xEventGroupCreate();
	xTaskCreate(CC_mainTask, "CC Main task", 100, NULL, 1, NULL ); //Crea task de misi�n
	state=CC_IDLE;
	prevState=CC_IDLE;
	currMission.active=0;
	xTaskCreate( CC_notifyStatus, "CC notify status task", 200	, NULL, 1, NULL );
}
