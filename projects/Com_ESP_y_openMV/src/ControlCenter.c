/*
 * ControlCenter.c
 *
 *  Created on: Sep 7, 2020
 *      Author: Javier
 */


#include "../inc/ControlCenter.h"
#include <string.h>

/*==================[defines]================================================*/

#define MSG_QUEST_HEADER "Quest?"

#define MSG_QUEST_ACCEPT "Quest?\nYes"

/*==================[typedef]================================================*/


/*==================[internal data declaration]==============================*/
static CC_State state;
static EthMsg msgRecieved,auxSendMsg;
static Mission currMission;


void CC_mainTask();
void parseEthMsg(EthMsg *  msg);
void decodeMission(char * questRecieved);
bool_t getNextMissionBlock(Mission_Block * mb);
bool_t isHeader(EthMsg * msg,char * header, char * p2Data);


/*==================[internal functions definition]==========================*/
void CC_mainTask()
{
	const TickType_t delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes

	for( ;; )
	{
		if(CCO_recieveMsg(&msgRecieved)) //Si llegó un mensaje del centro de comunicaciones se lo parsea
			parseEthMsg(&msgRecieved);
		else
			vTaskDelay(delay);
	}
}

void parseEthMsg(EthMsg * msgP)
{
	Mission_Block mb;
	char * p2Data; //Puntero a data de los misiones (lo que le sigue al header)
	if(state==CC_IDLE && isHeader(msgP,MSG_QUEST_HEADER,p2Data)) //En caso de que
	{
		decodeMission(p2Data);
		getNextMissionBlock(&mb);
		PC_setMissionBlock(mb);
		strcpy(auxSendMsg.array,MSG_QUEST_ACCEPT);
		CCO_sendMsg(auxSendMsg);
	}
}

bool_t isHeader(EthMsg * msg,char * header, char * p2Data)
{
	bool_t retVal= (msg->array == strstr (msg->array,header)); //Si el header es lo primero que llega el mensaje
	if(retVal) //Si hay header, devuelve el puntero al dato
		p2Data = msg->array + strlen(header); //El puntero a datos es donde terminan los headers
	return retVal;
}
void decodeMission(char *questRecieved)
{
	//Se encarga de pasar el string al formato que se guardaron los datos.
}
bool_t getNextMissionBlock(Mission_Block * mb)
{
	mb->md=currMission.blocks[currMission.currBlock];
	mb->com=BLOCK_START;
	return 1; //Quizás se podría fijar si había que esperar por el evento.
}

/*==================[external functions declaration]=========================*/

void CC_init()
{
	PC_Init();
	CCO_Init();
	xTaskCreate(CC_mainTask, "PC Main task", 512, NULL, CC_MAIN_PRIORITY, NULL ); //Crea task de misión
	state=CC_IDLE;
}
