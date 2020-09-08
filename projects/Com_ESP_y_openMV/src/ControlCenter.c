/*
 * ControlCenter.c
 *
 *  Created on: Sep 7, 2020
 *      Author: Javier
 */

#include "PathControl.h"
#include "CommunicationCentre.h"
#include "ControlCenter.h"
#include <string.h>

#define MSG_QUEST_HEADER "Quest?"

#define MSG_QUEST_ACCEPT "Quest?\nYes"


CC_State state;

void CC_mainTask();

void parseMsg(char * msg);
void decodeMission(char * questRecieved);
bool_t getNextMissionBlock(Mission_Block * mb);

bool_t isHeader(char * msg,char * header)
{
	return msg == strstr (msg,header); //Si el header es lo primero que llega el mensaje
}



void CC_mainTask()
{
	const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 ); //Delay para ver si tiene mensajes
	char * msgRecieved;
	for( ;; )
	{
		if(CCO_recieveMsg(msgRecieved)) //Si llegó un mensaje del centro de comunicaciones
			parseMsg(msgRecieved);
		else
			vTaskDelay( xDelay250ms );
	}
}

void parseMsg(char * msg)
{
	Mission_Block mb;
	if(state==CC_IDLE && isHeader(msg,MSG_QUEST_HEADER))
	{
		decodeMission(msg);
		getNextMissionBlock(&mb);
		PC_setMissionBlock(mb);
		CCO_sendMsg(MSG_QUEST_ACCEPT); //Esto puede estallar. Pensarlo bien
	}
}
void decodeMission(char * questRecieved)
{

}
bool_t getNextMissionBlock(Mission_Block * mb)
{
	return 1;
}

void CC_init()
{
	PC_Init();
	CCO_Init();
	xTaskCreate(CC_mainTask, "PC Main task", 512, NULL, 1, NULL ); //Crea task de misión
	state=CC_IDLE;
}
