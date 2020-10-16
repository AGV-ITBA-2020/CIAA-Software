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
		CC_mainFSM(ev); //Con el evento que llega se ejecuta la m�quina de estados
	}
}
void CC_mainFSM(EventBits_t ev)
{
	if(ev & GEG_COMS_RX){
		recHeader=CCO_getMsgType(); //Si fue un mensaje de internet, recibe el header
	}
	if(ev & GEG_HMI){
		//HMIeventType=?; //Si fue un evento de HMI, se deber�a obtener que tipo de evento fue
	}
	CC_indepParseEv(ev); //Eventos que no dependen de su estado inicial (van a error)
	switch(state){
		case CC_IDLE: //Funciones que dividen la FSM dependiendo del estado en el que se est�
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
	if(ev == 0) //Si hubo un timeout. En principio no pasar�a nada no?
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
		if(!currMission.waitForInterBlockEvent) //En el caso que no necesite un evento extra para arrancar la misi�n
		{
			state=CC_ON_MISSION; //Pasa a estado misi�n
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
		else if(!isMissionCompleted()) //Sino, si todav�a no termin� la misi�n le paso el bloque que viene
		{
			//PC_setMissionBlock(getNextMissionBlock());
			xEventGroupSetBits( xEventGroup, GEG_MISSION_STARTED);
		}
		else //En caso de que ya la haya terminado, voy a IDLE
			state=CC_IDLE;
	}
	if(ev & (GEG_EMERGENCY_STOP | GEG_PRIORITY_STOP )) //Cualquier emergencia va a estado de emergencia
		state=CC_PAUSE_EMERGENCY;
	if((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) //Recibe que aborte misi�n por internes
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		state=CC_IDLE;
	}

	if((ev & GEG_COMS_RX) && recHeader == CCO_PAUSE_MISSION)//Recibe que pause misi�n por internes
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_PAUSE_CMD);
		state=CC_PAUSE_EMERGENCY;
	}


}
void CC_pauseEmergencyParseEv(EventBits_t ev)
{
	/*Esto ser�a correspondiente a pausa */
	if(currMission.waitForInterBlockEvent) //En el caso en que se este esperando por un evento entre bloques.
	{
		if(((currMissionIBE()== IBE_HOUSTON_CONTINUE) && (ev & GEG_COMS_RX) && (recHeader == CCO_CONTINUE)) || //Si se esperaba un continue de houston y lleg�
		   ((currMissionIBE()== IBE_BUTTON_PRESS) && (ev & GEG_HMI) /*&& (ev & GEG_HMI) && HMIevType==LONG_PRESS */)){ //o si se esperaba presionar un bot�n y se presion�
			currMission.waitForInterBlockEvent=false; //Ya no se tiene que esperar por evento.
			CCO_sendMsgWithoutData(CCO_IBE_RECIEVED); //Comunica a Houston que recibi� el IBE
			if(!isMissionCompleted()) //Si faltan bloques de la misi�n
			{
				//PC_setMissionBlock(getNextMissionBlock());
				xEventGroupSetBits( xEventGroup, GEG_MISSION_STARTED);
				state=CC_ON_MISSION;
			}
			else //Si este evento era el �ltimo paso de la misi�n, vuelve a IDLE
				state=CC_IDLE;
		}
	}
	if((ev & GEG_COMS_RX) && recHeader == CCO_ABORT_MISSION) //Si estando en pausa recibe que aborte misi�n.
	{
		xEventGroupSetBits( xEventGroup, GEG_MISSION_ABORT_CMD);
		state=CC_IDLE;
	}
	/*Esto ser�a correspondiente a emergencia */
	if((ev & GEG_COMS_RX) && recHeader == CCO_CONTINUE /*&& (ev & GEG_HMI) && HMIevType==LONG_PRESS */)
	{
//		if(chekcOnEmergency()) //Se fija que no siga estando en emergencia
//		{
//			state = prevState;
			//xEventGroupSetBits( xEventGroup, GEG_MISSION_CONTINUE_CMD); //Si vuelve a estado misi�n deber�a poner esto
//		}
	}
}

void CC_onErrorRoutine(EventBits_t ev)
{
	//Pasar por todos los m�dulos con eventos de error y recupera el string de error
	//Lo comunica a houston salvo que sea algo del centro de comunicaciones (CCO_sendError(string err))
}

/*Estas funciones de abajo le dar�an motivo a una clase misi�n (con funciones propias) */
void missionAdvance()
{
	currMission.currBlock++; //Avanza el bloque
	if(currMission.interBlockEvent[currMission.currBlock]==IBE_NONE) //Se fija si necesita un evento para avanzar al siguiente bloque
		currMission.waitForInterBlockEvent=false;
	else
		currMission.waitForInterBlockEvent=true;
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
void CC_init()
{
	PC_Init();
	CCO_init();
	xEventGroup =  xEventGroupCreate();
	xTaskCreate(CC_mainTask, "CC Main task", 100, NULL, 1, NULL ); //Crea task de misi�n
	state=CC_IDLE;
	prevState=CC_IDLE;
	currMission.active=0;
}
