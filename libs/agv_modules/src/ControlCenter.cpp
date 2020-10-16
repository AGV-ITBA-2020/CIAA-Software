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

typedef enum{CC_IDLE,CC_ON_MISSION,CC_MANUAL, CC_ERROR, CC_PAUSE_EMERGENCY, CC_LOWPOWER} CC_State;

/*==================[internal data declaration]==============================*/
extern EventGroupHandle_t xEventGroup;
static CC_State state,prevState;
static MISSION_T currMission;
static MSG_REC_HEADER_T recHeader;

void CC_mainTask(void *);
void CC_mainFSM(EventBits_t ev);

void CC_onErrorRoutine(EventBits_t ev);
void CC_indepParseEv(EventBits_t ev);
void CC_onMissionParseEv(EventBits_t ev);
void CC_idleParseEv(EventBits_t ev);
void CC_pauseEmergencyParseEv(EventBits_t ev);

INTER_BLOCK_EVENT_T currMissionIBE();
BLOCK_DETAILS_T * getNextMissionBlock();
bool_t isMissionCompleted();
void missionAdvance();
/*==================[internal functions definition]==========================*/
void CC_mainTask(void *)
{
	const TickType_t timeoutDelay = pdMS_TO_TICKS( 12000 );
	while(!CCO_connected());
	for( ;; )
	{
		EventBits_t ev = xEventGroupWaitBits( xEventGroup,CC_EVENT_MASK,pdTRUE,pdFALSE,timeoutDelay );
		CC_mainFSM(ev); //Con el evento que llega se ejecuta la máquina de estados
	}
}
void CC_mainFSM(EventBits_t ev)
{
	if(ev & GEG_COMS_RX){
		recHeader=CCO_getMsgType(); //Si fue un mensaje de internet, recibe el header
	}
	if(ev & GEG_HMI){
		//HMIeventType=?; //Si fue un evento de HMI, se debería obtener que tipo de evento fue
	}
	CC_indepParseEv(ev); //Eventos que no dependen de su estado inicial (van a error)
	switch(state){
		case CC_IDLE: //Funciones que dividen la FSM dependiendo del estado en el que se esté
			CC_idleParseEv(ev);
			break;
		case CC_ON_MISSION:
			CC_onMissionParseEv(ev);
			break;
		case CC_PAUSE_EMERGENCY:
			CC_pauseEmergencyParseEv(ev);
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
		state=CC_ERROR;
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
		//PC_setMissionBlock(getNextMissionBlock());
		if(!currMission.waitForInterBlockEvent) //En el caso que no necesite un evento extra para arrancar la misiï¿½n
		{
			state=CC_ON_MISSION; //Pasa a estado misiï¿½n
			xEventGroupSetBitsFromISR( xEventGroup, GEG_MISSION_STARTED, NULL );
		}
		else
			state=CC_PAUSE_EMERGENCY; //Sino espera hasta que ocurra ese evento
	}
//	if((ev & GEG_HMI) && HMIevType==MANUAL_MODE)
//		state=CC_MANUAL;
//
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
		if(!currMission.waitForInterBlockEvent) //Si se tiene que esperar por un evento se va a pausa
			state=CC_PAUSE_EMERGENCY;
		else if(!isMissionCompleted()) //Sino, si todavía no terminó la misión le paso el bloque que viene
		{
			//PC_setMissionBlock(getNextMissionBlock());
			xEventGroupSetBits( xEventGroup, GEG_MISSION_STARTED);
		}
		else //En caso de que ya la haya terminado, voy a IDLE
			state=CC_IDLE;
	}
	if(ev & (GEG_EMERGENCY_STOP | GEG_PRIORITY_STOP )) //Cualquier emergencia va a estado de emergencia
		state=CC_PAUSE_EMERGENCY;
	if((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) //Recibe que aborte misión por internes
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		state=CC_IDLE;
	}

	if((ev & GEG_COMS_RX) && recHeader == CCO_PAUSE_MISSION)//Recibe que pause misión por internes
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_PAUSE_CMD);
		state=CC_PAUSE_EMERGENCY;
	}


}
void CC_pauseEmergencyParseEv(EventBits_t ev)
{
	/*Esto sería correspondiente a pausa */
	if(currMission.waitForInterBlockEvent) //En el caso en que se este esperando por un evento entre bloques.
	{
		if(((currMissionIBE()== IBE_HOUSTON_CONTINUE) && (ev & GEG_COMS_RX) && (recHeader == CCO_CONTINUE)) || //Si se esperaba un continue de houston y llegó
		   ((currMissionIBE()== IBE_BUTTON_PRESS) && (ev & GEG_HMI) /*&& (ev & GEG_HMI) && HMIevType==LONG_PRESS */)){ //o si se esperaba presionar un botón y se presionó
			currMission.waitForInterBlockEvent=false; //Ya no se tiene que esperar por evento.
			CCO_sendMsgWithoutData(CCO_IBE_RECIEVED); //Comunica a Houston que recibió el IBE
			if(!isMissionCompleted()) //Si faltan bloques de la misión
			{
				//PC_setMissionBlock(getNextMissionBlock());
				xEventGroupSetBits( xEventGroup, GEG_MISSION_STARTED);
				state=CC_ON_MISSION;
			}
			else //Si este evento era el último paso de la misión, vuelve a IDLE
				state=CC_IDLE;
		}
	}
	if((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) //Si estando en pausa recibe que aborte misión.
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		state=CC_IDLE;
	}
	/*Esto sería correspondiente a emergencia */
	if((ev & GEG_COMS_RX) && recHeader == CCO_CONTINUE /*&& (ev & GEG_HMI) && HMIevType==LONG_PRESS */)
	{
//		if(chekcOnEmergency()) //Se fija que no siga estando en emergencia
//		{
//			state = prevState;
			//xEventGroupSetBits( xEventGroup, GEG_MISSION_CONTINUE_CMD); //Si vuelve a estado misión debería poner esto
//		}
	}
}

void CC_onErrorRoutine(EventBits_t ev)
{
	//Pasar por todos los módulos con eventos de error y recupera el string de error
	//Lo comunica a houston salvo que sea algo del centro de comunicaciones (CCO_sendError(string err))
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
}
