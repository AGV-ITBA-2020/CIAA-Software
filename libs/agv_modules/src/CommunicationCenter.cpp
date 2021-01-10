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
#include <ctype.h>
#include "GlobalEventGroup.h"



extern EventGroupHandle_t xEventGroup;
static EthMsg auxRecMsg,auxSendMsg;

void msgRecCallback(void *);
string getHeader(string auxRecStr);
string getData(string auxRecStr);

bool_t isCheckpoint(string ethFormat);
bool_t isIBE(string ethFormat);
BLOCK_CHECKPOINT_T getCheckpoint(string ethFormat);
INTER_BLOCK_EVENT_T getIBE(string ethFormat);


/*==================[internal functions definition]==========================*/


string getHeader(string auxRecStr)
{
	return auxRecStr.substr(0,auxRecStr.find_first_of('\n'));
}

string getData(string auxRecStr)
{
	return auxRecStr.substr(auxRecStr.find_first_of('\n')+1);
}


/*==================[external functions declaration]=========================*/

void CCO_init()
{
	EMH_init(msgRecCallback,NULL);
}
bool_t CCO_connected()
{
	return EMH_connected();
}

MSG_REC_HEADER_T CCO_getMsgType()
{
	MSG_REC_HEADER_T retVal;

	//map<string,MSG_REC_HEADER_T> recHeaderLUT= { {"Quest?",CCO_NEW_MISSION},{"Continue",CCO_CONTINUE_MISSION},{"Status",CCO_STATUS_REQ},{"Quest?",CCO_NEW_MISSION},{"QuestAbort?",CCO_ABORT_MISSION},{"Pause",CCO_PAUSE_MISSION},{"Fixed speed",CCO_SET_VEL}}; //Estp se ve que no le gustï¿½
//	if(recHeaderLUT.count(header)) //Si el header existe
//		retVal=recHeaderLUT[header]; //Devuelvo el tipo que se corresponde con ese header

	if(!EMH_recieveMsg(&auxRecMsg))
		assert(0); //En caso que se llamo a esta funcion pero la capa inferior no tenï¿½a mensajes
	string auxRecStr = auxRecMsg.array;
	string header = getHeader(auxRecStr);

	if(header=="Quest?")
		retVal=CCO_NEW_MISSION;
	else if(header =="Continue")
		retVal=CCO_CONTINUE;
	else if(header =="Status")
		retVal=CCO_STATUS_REQ;
	else if(header =="QuestAbort?")
		retVal=CCO_ABORT_MISSION;
	else if(header =="Pause")
		retVal=CCO_PAUSE_MISSION;
	else if(header =="Fixed speed")
		retVal=CCO_SET_VEL;
	else if(header =="Set K PID")
		retVal=CCO_SET_K_PID;
	else
		retVal=CCO_NOT_DEF;

	return retVal;
}

bool_t CCO_getMission(MISSION_T * mission)
{
	bool_t retVal=1;
	string auxRecStr = auxRecMsg.array;
	string data = getData(auxRecStr); //BUTTON_PRESS,HOUSTON_CONTINUE, DELAY, NONE
	//map<string,BLOCK_CHECKPOINT_T> checkpointLUT = { {"Sd",CHECKPOINT_SLOW_DOWN}, {"Su",CHECKPOINT_SPEED_UP}, {"Fr",CHECKPOINT_FORK_RIGHT},{"Fl",CHECKPOINT_FORK_LEFT},{"St",CHECKPOINT_STATION},{"Me",CHECKPOINT_MERGE}};
	//map<string,INTER_BLOCK_EVENT_T> interBlockLUT = { {"Bp",BUTTON_PRESS}, {"Hc",HOUSTON_CONTINUE}, {"De",DELAY},{"No",NONE}};
	mission->nmbrOfBlocks=0;
	unsigned int i=0;
	while(i < data.size())
	{

		if(isdigit(data[i])) //Si es un nï¿½mero (que representa una distancia)
		{
			mission->blocks[mission->nmbrOfBlocks].distances[mission->blocks[mission->nmbrOfBlocks].blockLen]=stoi(data.substr(i));
			while(isdigit(data[i])) //Salteo hasta que se terminen los nï¿½meros
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
			else if(isIBE(com)) //Si es un evento entre bloques
				mission->interBlockEvent[mission->nmbrOfBlocks]=getIBE(com);
			else if(isCheckpoint(com)) //Si es un checkpoint
				mission->blocks[mission->nmbrOfBlocks].blockCheckpoints[(mission->blocks[mission->nmbrOfBlocks].blockLen)++]=getCheckpoint(com);
			else
				assert(0); //Una misiï¿½n no puede tener un campo que no sea los mencionados
			i+=2;
		}
	}
	mission->currBlock=0;
	return retVal;
}

bool_t isCheckpoint(string e)
{
	return (e=="Sd") || (e=="Su") || (e=="Fr") || (e=="Fl") || (e=="Me") || (e=="St");
}
bool_t isIBE(string e)
{
	return (e=="Bp") || (e=="Hc") || (e=="De") || (e=="No") ;
}
BLOCK_CHECKPOINT_T getCheckpoint(string e)
{
	BLOCK_CHECKPOINT_T retVal;
	if(e=="Sd")
		retVal=CHECKPOINT_SLOW_DOWN;
	else if(e=="Su")
		retVal=CHECKPOINT_SPEED_UP;
	else if(e=="Fr")
		retVal=CHECKPOINT_FORK_RIGHT;
	else if(e=="Fl")
		retVal=CHECKPOINT_FORK_LEFT;
	else if(e=="Me")
		retVal=CHECKPOINT_MERGE;
	else if(e=="St")
		retVal=CHECKPOINT_STATION;
	return retVal;
}
INTER_BLOCK_EVENT_T getIBE(string e)
{
	INTER_BLOCK_EVENT_T retVal;
	if(e=="Bp")
		retVal=IBE_BUTTON_PRESS;
	else if(e=="Hc")
		retVal=IBE_HOUSTON_CONTINUE;
	else if(e=="De")
		retVal=IBE_DELAY;
	else if(e=="No")
		retVal=IBE_NONE;
	return retVal;
}

double CCO_getLinSpeed()
{
	string auxStr = auxRecMsg.array;
	auxStr=getData(auxStr);
	return stoi(auxStr)*0.1;
}

double CCO_getAngSpeed()
{
	string auxStr = auxRecMsg.array;
	auxStr=getData(auxStr);
	return stoi(auxStr.substr(auxStr.find_first_of(' ')+1))*0.01;
}
PID_KS CCO_GetPIDKs()
{
	PID_KS retVal;
	string auxStr = auxRecMsg.array;
	auxStr=getData(auxStr);
	retVal.Kp = stof(auxStr);
	auxStr=auxStr.substr(auxStr.find_first_of(' ')+1);
	retVal.Ki = stof(auxStr);
	auxStr=auxStr.substr(auxStr.find_first_of(' ')+1);
	retVal.Kd = stof(auxStr);
	return retVal;
}

bool_t CCO_sendMsgWithoutData(MSG_SEND_HEADER_T msg)
{
	bool_t retVal= 1;
	//map<MSG_SEND_HEADER_T,string> msgLUT = { {CCO_MISSION_ACCEPT,"Quest\nYes"},{CCO_MISSION_DENY,"Quest\nNo"},{CCO_MISSION_STEP_REACHED,"Quest step reached"}};
	string auxStr;
	if(msg == CCO_MISSION_ACCEPT)
		auxStr="Quest\nYes";
	else if (msg == CCO_MISSION_DENY)
		auxStr="Quest\nNo";
	else if (msg == CCO_MISSION_STEP_REACHED)
		auxStr="Quest step reached";
	else if (msg == CCO_IBE_RECIEVED)
		auxStr="Interblock event recieved";
	else if (msg == CCO_EMERGENCY_STOP)
		auxStr="Emergency";
	else if (msg == CCO_PRIORITY_STOP)
		auxStr="Quest paused\nPriority stop";
	else if (msg == CCO_MISSION_ABORT)
		auxStr="Quest abort";
	else if (msg == CCO_MISSION_PAUSE)
			auxStr="Quest paused";
	else if (msg == CCO_CONTINUE_SEND)
		auxStr="Continue";
	else
		assert(0);

	auxStr.copy(auxSendMsg.array,auxStr.length());
	auxSendMsg.array[auxStr.length()] = 0;
	retVal= EMH_sendMsg(&auxSendMsg);
	return retVal;
}

bool_t CCO_sendStatus(AGV_STATUS_T status)
{
	bool_t retVal=0;
	string header= "Status\n";
	string distance= "Distance: "+ to_string(int(status.distEst));
	string batVolt= "\nBatVolt: "+ to_string(int(100*status.batVoltage));
	//Acá vendrán más status que aporten, pero por ahora se necesitaban estos.
	string finalMsg= header + distance + batVolt;
	finalMsg.copy(auxSendMsg.array,finalMsg.length());
	retVal= EMH_sendMsg(&auxSendMsg);
	return retVal;
}

bool_t CCO_sendError(string err)
{
	bool_t retVal=1;
	string finalMsg= "Error\n"+err;
	finalMsg.copy(auxSendMsg.array,finalMsg.length());
	retVal= EMH_sendMsg(&auxSendMsg);
	return retVal;
}
/*==================[interrupt callbacks]==========================*/

void msgRecCallback(void *)
{
	xEventGroupSetBitsFromISR( xEventGroup, GEG_COMS_RX, NULL );
}

