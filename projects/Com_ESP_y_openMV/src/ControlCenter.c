/*
 * ControlCenter.c
 *
 *  Created on: Sep 7, 2020
 *      Author: Javier
 */


#include "../inc/ControlCenter.h"
#include <string.h>

#include "../inc/PathControl.h"
#include "../inc/CommunicationCentre.h"
#include "task.h"


/*==================[defines]================================================*/

#define MSG_QUEST_HEADER "Quest?"

#define MSG_QUEST_ACCEPT "Quest?\nYes"
#define MSG_QUEST_STEP_REACHED "Quest step reached"

/*==================[typedef]================================================*/

typedef struct{
	Block_details blocks[MAX_NMBR_OF_BLOCKS_IN_MISSION]; //Bloques de la misi�n
	intraBlockEvent intraBlockEvent[MAX_NMBR_OF_BLOCKS_IN_MISSION]; //que evento trigerea el siguiente bloque
	bool_t active,waitForIntraBlockEvent;
	unsigned int currBlock,nmbrOfBlocks;
	//M�s metadata. Para probar no es nada m�s necesario.
} Mission;

/*==================[internal data declaration]==============================*/
static CC_State state;
static EthMsg msgRecieved,auxSendMsg;
static Mission currMission;






void CC_mainTask();
void parseEthMsg(EthMsg *  msg);
void decodeMission(char * questRecieved);
bool_t getNextMissionBlock(Mission_Block * mb);
bool_t isHeader(EthMsg * msg,char * header, char * p2Data);
void parsePCEvent(PC_Event ev);
void sendESPStr(char * msg);


/*==================[internal functions definition]==========================*/
void CC_mainTask()
{
	const TickType_t delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes

	for( ;; )
	{
		if(CCO_recieveMsg(&msgRecieved)) //Si lleg� un mensaje del centro de comunicaciones se lo parsea
			parseEthMsg(&msgRecieved);
		if(PC_hasEvent())
			parsePCEvent(PC_getEvent());

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
		sendESPStr(MSG_QUEST_ACCEPT);
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
	currMission.active=1;
	//Se encarga de pasar el string al formato que se guardaron los datos.
}
bool_t getNextMissionBlock(Mission_Block * mb)
{
	mb->md=currMission.blocks[currMission.currBlock];
	mb->com=BLOCK_START;
	return 1; //Quiz�s se podr�a fijar si hab�a que esperar por el evento.
}
void parsePCEvent(PC_Event ev)
{
	if(ev == PC_STEP_REACHED)
		sendESPStr(MSG_QUEST_STEP_REACHED);
	else if(ev == PC_BLOCK_FINISHED)
	{
		currMission.currBlock++;
		if(currMission.currBlock < currMission.nmbrOfBlocks) //Si todav�a quedan bloques por ejecutar
			currMission.waitForIntraBlockEvent=1; //Se pone en un estado para esperar el evento que le indique que siga la misi�n
	}
}
void sendESPStr(char * msg)
{
	strcpy(auxSendMsg.array,msg);
	CCO_sendMsg(&auxSendMsg);
}
/*==================[external functions declaration]=========================*/

void CC_init()
{
	PC_Init();
	CCO_init();
	xTaskCreate(CC_mainTask, "PC Main task", 512, NULL, CC_MAIN_PRIORITY, NULL ); //Crea task de misi�n
	state=CC_IDLE;
	currMission.active=0;
}