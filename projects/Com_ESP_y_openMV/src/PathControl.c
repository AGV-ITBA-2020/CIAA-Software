/*
 * RoadProcessing.c
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */

#include "../inc/my_sapi_uart.h"
#include "../inc/PathControl.h"
#include "../inc/MovementControl.h"

#include "event_groups.h"
#include "semphr.h"



/*==================[typedef]================================================*/
#define OPEN_MV_MSG_LEN 4 //Length en bytes del mensaje del openMV
#define MAX_DISPLACEMENT 64
#define EVENT_QUEUE_LEN 10

#define LOW_SPEED_VEL 0.5
#define HIGH_SPEED_VEL 1.0



typedef enum {OPENMV_FOLLOW_LINE, OPENMV_FORK_LEFT, OPENMV_FORK_RIGHT, OPENMV_MERGE, OPENMV_ERROR,OPENMV_IDLE}openMV_states; //Los distintos estados del OpenMV
typedef enum {TAG_SLOW_DOWN, TAG_SPEED_UP, TAG_STATION=3}Tag_t; //Los distintos TAGs

#define IS_FORKORMERGE_MISSION(x) ((x)==CHECKPOINT_FORK_LEFT || (x) == CHECKPOINT_FORK_RIGHT || (x) == CHECKPOINT_MERGE )

typedef struct  {
  int displacement;
  bool_t tag_found;
  bool_t form_passed;
  bool_t error;
  Tag_t tag;
}openMV_msg; //La información que trae cada mensaje del openMV.


/*==================[internal data declaration]==============================*/
static char recBuff[OPEN_MV_MSG_LEN];//Acá se guarda la data en la interrupción
static Mission_Block mb;
static SemaphoreHandle_t xBinarySemaphore;
static TaskHandle_t * missionTask;
static QueueHandle_t missionMailbox, eventQueue;
static float currVel=0;

void PC_MainTask();
openMV_msg parse_openmv_msg(char * buf);
void send_openmv_nxt_state(Block_checkpoint ms);
void PC_MissionTask();
void startNewMissionBlock();
void abortMissionBlock();
void callbackInterrupt(void *);
bool_t missionBlockLogic(openMV_msg msg, bool_t * stepReached); //Ejecuta la lógica de recorrida de camino, y devuelve si terminó la misión
float computeAngVel(unsigned int displacement); //Obtiene la velocidad angular objetivo (Acá debería estar el PID).
/*==================[internal functions definition]==========================*/


/*******Tasks*********/
void PC_MainTask()
{
	const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
	for( ;; )
	{
		xQueueReceive(missionMailbox,&mb,portMAX_DELAY); //Espera a que llegue una misión. Habría que poner timeout quizás?
		if(mb.com== BLOCK_START || mb.com==BLOCK_REPLACE)
			startNewMissionBlock();
		else
			abortMissionBlock();
	}


}
void PC_MissionTask()
{
	openMV_msg msg;
	bool_t quit,stepReached;
	PC_Event ev;
	for( ;; )
	{
		xSemaphoreTake( xBinarySemaphore, portMAX_DELAY ); //Espera hasta que haya nuevos datos del openMV
		stepReached=0;
		msg=parse_openmv_msg(recBuff); //Se decodifica el msg
		quit=missionBlockLogic(msg, &stepReached);
		MC_setLinVel(currVel);
		MC_setAngVel(computeAngVel(msg.displacement));
		if(stepReached)
		{
			ev=PC_STEP_REACHED;
			xQueueSendToBack(eventQueue, &ev,0);
		}
		if(quit)
		{
			ev=PC_BLOCK_FINISHED;
			xQueueSendToBack(eventQueue, &ev,0);
			abortMissionBlock();
		}
	}
}




/*******Otros*********/
bool_t missionBlockLogic(openMV_msg msg, bool_t *stepReached)
{
	Block_checkpoint currChkpnt = mb.md.blockCheckpoints[mb.md.currStep];
	Block_checkpoint nextChkpnt;
	bool_t missionFinished,stepsLeft=1;

	if(mb.md.currStep == mb.md.blockLen-1)
		stepsLeft = 0;
	else
		nextChkpnt= mb.md.blockCheckpoints[mb.md.currStep+1];//Si quedan pasos próximos, obtengo el proximo paso de la misión

	bool_t steppingCondition = ( 	((currChkpnt == CHECKPOINT_SLOW_DOWN) && msg.tag_found && msg.tag==TAG_SLOW_DOWN) 	||
									((currChkpnt == CHECKPOINT_SPEED_UP) && msg.tag_found && msg.tag==TAG_SPEED_UP) 	||
									((currChkpnt == CHECKPOINT_STATION) && msg.tag_found && msg.tag==TAG_STATION) 		||
									(IS_FORKORMERGE_MISSION(currChkpnt) && msg.form_passed)
								); //Condiciones para ir al siguiente paso de la misión
	//Control de pasos de misión
	if (steppingCondition)
	{
		mb.md.currStep++;
		*stepReached=1;
		if(stepsLeft)
			send_openmv_nxt_state(nextChkpnt);
	}
	//Control de velocidad
	if ((currChkpnt == CHECKPOINT_SLOW_DOWN) && msg.tag_found && msg.tag==TAG_SLOW_DOWN)
		currVel=LOW_SPEED_VEL;
	if ((currChkpnt == CHECKPOINT_SPEED_UP) && msg.tag_found && msg.tag==TAG_SPEED_UP)
		currVel=HIGH_SPEED_VEL;
	if(mb.md.currStep == mb.md.blockLen-1)
		missionFinished=1;
	return missionFinished;
}
void startNewMissionBlock()
{
	currVel=HIGH_SPEED_VEL;
	for(unsigned int i=0; i<OPEN_MV_MSG_LEN; i++ )
		uartReadByte(PC_UART,(uint8_t *) &(recBuff[i]));// Si quedó basura en la cola de la uart la vuela
	uartCallbackSet( PC_UART, UART_RECEIVE,(callBackFuncPtr_t) callbackInterrupt);
	xTaskCreate( PC_MissionTask, "RP mission task", 100	, NULL, 2, missionTask ); //Crea task de misión
	uartInterrupt( PC_UART, 1 ); //Enables uart interrupts
}
void abortMissionBlock()
{
	currVel=0;
	MC_setLinVel(currVel);
	uartInterrupt( PC_UART, 0 ); //Disables interrupts
	uartTxWrite(PC_UART,OPENMV_IDLE);
	if(missionTask!=NULL)
		vTaskDelete(missionTask); //Borra la task de misión

}
openMV_msg parse_openmv_msg(char * buf)
{
	openMV_msg retVal;
	retVal.displacement=buf[0];
	retVal.error= (buf[1] /128)%2;
	retVal.form_passed= (buf[1]/64)%2;
	retVal.tag_found= (buf[1]/32)%2;
	retVal.tag=buf[1]%32;
	return retVal;
}
void send_openmv_nxt_state(Block_checkpoint ms)
{ //Correlación entre los estados del openmv y el checkpoint que viene.
	if(ms == CHECKPOINT_FORK_LEFT)
		uartTxWrite(PC_UART,OPENMV_FORK_LEFT);
	else if(ms == CHECKPOINT_FORK_RIGHT)
		uartTxWrite(PC_UART,OPENMV_FORK_RIGHT);
	else if(ms == CHECKPOINT_STATION || ms == CHECKPOINT_SLOW_DOWN || ms==CHECKPOINT_SPEED_UP)
		uartTxWrite(PC_UART,OPENMV_FOLLOW_LINE);
	else if(ms == CHECKPOINT_MERGE)
		uartTxWrite(PC_UART,OPENMV_MERGE);
}

void callbackInterrupt(void* a)
{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //Seccion 6.4 semáforos binarios freeRTOS
	for(unsigned int i=0; i<OPEN_MV_MSG_LEN; i++ )
		uartReadByte(PC_UART,(uint8_t *) &(recBuff[i]));// Read from RX FIFO
	xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); //Esto no lo entiendo bien
}

float computeAngVel(unsigned int displacement) //Desarrollar con PID
{
	float a;
	return a;
}
/*==================[external functions declaration]=========================*/

void PC_Init(void)
{
	uartInit( PC_UART, PC_UART_BAUDRATE, 1 );
	MC_init();
	xBinarySemaphore = xSemaphoreCreateBinary();
	xTaskCreate( PC_MainTask, "PC Main task", 100	, NULL, 1, NULL ); //Crea task de misión
	missionMailbox=xQueueCreate( 1,sizeof(Mission_Block));
	eventQueue=xQueueCreate( EVENT_QUEUE_LEN,sizeof(PC_Event));
}

void PC_setMissionBlock(Mission_Block mb)
{
	xQueueSendToBack(missionMailbox,&mb,0 );
}

bool_t PC_hasEvent()
{
	return uxQueueMessagesWaiting(eventQueue);
}

PC_Event PC_getEvent()
{
	PC_Event retVal;
	xQueueReceive( eventQueue,&retVal,0);
	return retVal;
}
