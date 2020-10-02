/*
 * CommunicationCenter.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: Javier
 */

#include "EthMsgHandler.h"
#include "CommunicationCenter.hpp"
#include "config.h"
#include "assert.h"
#include <string>
#include <map>
#include <ctype.h>

using namespace std;


static EventGroupHandle_t eventGroup;
static EthMsg auxRecMsg,auxSendMsg;

void msgRecCallback(void *);
string getHeader(string auxRecStr);
string getData(string auxRecStr);


/*==================[internal functions definition]==========================*/


string getHeader(string auxRecStr)
{
	return auxRecStr.substr(0,auxRecStr.find_first_of('\n'));
}

string getData(string auxRecStr)
{
	return auxRecStr.substr(auxRecStr.find_first_of('\n'));
}


/*==================[external functions declaration]=========================*/

void CCO_init(EventGroupHandle_t xEventGroup)
{
	eventGroup=xEventGroup;
	EMH_init(msgRecCallback,NULL);
}

MSG_REC_HEADER_T CCO_getMsgType()
{
	MSG_REC_HEADER_T retVal;

	map<string,MSG_REC_HEADER_T> recHeaderLUT= { {"Quest?",CCO_NEW_MISSION},{"Continue",CCO_CONTINUE_MISSION},{"Status",CCO_STATUS_REQ},{"Quest?",CCO_NEW_MISSION},{"QuestAbort?",CCO_ABORT_MISSION},{"Pause",CCO_PAUSE_MISSION}};

	if(!EMH_recieveMsg(&auxRecMsg))
		assert(0); //En caso que se llamo a esta funcion pero la capa inferior no tenía mensajes
	string auxRecStr = auxRecMsg.array;
	string header = getHeader(auxRecStr);

	if(recHeaderLUT.count(header)) //Si el header existe
		retVal=recHeaderLUT[auxRecStr]; //Devuelvo el tipo que se corresponde con ese header
	else
		retVal=CCO_NOT_DEF;

	return retVal;
}

bool_t CCO_getMission(MISSION_T * mission)
{
	bool_t retVal=1;
	string auxRecStr = auxRecMsg.array;
	string data = getData(auxRecStr); //BUTTON_PRESS,HOUSTON_CONTINUE, DELAY, NONE
	map<string,BLOCK_CHECKPOINT_T> checkpointLUT = { {"Sd",CHECKPOINT_SLOW_DOWN}, {"Su",CHECKPOINT_SPEED_UP}, {"Fr",CHECKPOINT_FORK_RIGHT},{"Fl",CHECKPOINT_FORK_LEFT},{"St",CHECKPOINT_STATION},{"Me",CHECKPOINT_MERGE}};
	map<string,INTER_BLOCK_EVENT_T> interBlockLUT = { {"Bp",BUTTON_PRESS}, {"Hc",HOUSTON_CONTINUE}, {"De",DELAY},{"No",NONE}};
	mission->nmbrOfBlocks=0;
	//FALTA PONER LAS DISTANCIAS!!!!!!!!!!!!!!!!!!!!!!!!!!!
	unsigned int i=0;
	while(i < data.size())
	{

		if(isdigit(data[i])) //Si es un número (que representa una distancia)
		{
			mission->blocks[mission->nmbrOfBlocks].distances[mission->blocks[mission->nmbrOfBlocks].blockLen]=stoi(data.substr(i));
			while(isdigit(data[i])) //Salteo hasta que se terminen los números
				i++;
		}
		else //Sino, tomo los comandos y los parseo como son.
		{
			string com = data.substr(i,2);
			if(com == "Bs")
			{
				mission->blocks[mission->nmbrOfBlocks].blockLen=0;
				mission->blocks[mission->nmbrOfBlocks].currStep=0;
			}
			else if(com == "Be")
				(mission->nmbrOfBlocks)++;
			else if(interBlockLUT.count(com)) //Si es un evento entre bloques
				mission->interBlockEvent[mission->nmbrOfBlocks]=interBlockLUT[com];
			else if(checkpointLUT.count(com)) //Si es un checkpoint
				mission->blocks[mission->nmbrOfBlocks].blockCheckpoints[(mission->blocks[mission->nmbrOfBlocks].blockLen)++]=checkpointLUT[com];
			else
				assert(0); //Una misión no puede tener un campo que no sea los mencionados
			i+=2;
		}


	}
	return retVal;
}

bool_t CCO_sendMsgWithoutData(MSG_SEND_HEADER_T msg)
{
	bool_t retVal= 0;
	map<MSG_SEND_HEADER_T,string> msgLUT = { {CCO_MISSION_ACCEPT,"Quest\nYes"},{CCO_MISSION_DENY,"Quest\nNo"},{CCO_MISSION_STEP_REACHED,"Quest step reached"}};
	if(msgLUT.count(msg))
	{
		string str = msgLUT[msg];
		str.copy(auxSendMsg.array,str.length());
		retVal= EMH_sendMsg(&auxSendMsg);
	}
	return retVal;
}

bool_t CCO_sendStatus(AGV_STATUS_T status)
{
	bool_t retVal=0;

	return retVal;
}

/*==================[interrupt callbacks]==========================*/

void msgRecCallback(void *)
{
	xEventGroupSetBitsFromISR( eventGroup, EV_CCO_MSG_REC, NULL );
}

