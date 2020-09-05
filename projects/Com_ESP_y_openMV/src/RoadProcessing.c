/*
 * RoadProcessing.c
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */

#include "RoadProcessing.h"
#include "../inc/my_sapi_uart.h"

#include "event_groups.h"
#include "semphr.h"


/*==================[typedef]================================================*/
#define OPEN_MV_MSG_LEN 4 //Length en bytes del mensaje del openMV
#define MAX_DISPLACEMENT 64


typedef enum {OPENMV_FOLLOW_LINE, OPENMV_FORK_LEFT, OPENMV_FORK_RIGHT, OPENMV_MERGE, OPENMV_ERROR,OPENMV_IDLE}openMV_states; //Los distintos estados del OpenMV
typedef enum {TAG_SLOW_DOWN, TAG_SPEED_UP, TAG_STATION=3}Tag_t; //Los distintos TAGs

#define IS_FORKORMERGE_MISSION(x) ((x)==MISSION_FORK_LEFT || (x) == MISSION_FORK_RIGHT || (x) == MISSION_MERGE )

typedef struct  {
  int displacement;
  bool_t tag_found;
  bool_t form_passed;
  bool_t error;
  Tag_t tag;
}openMV_msg; //La información que trae cada mensaje del openMV.


/*==================[internal data declaration]==============================*/
static char recBuff[OPEN_MV_MSG_LEN];//Acá se guarda la data en la interrupción
static Mission_struct ms;
static bool_t queuesAttached=0;
static SemaphoreHandle_t xBinarySemaphore;
static TaskHandle_t * missionTask;
static QueueHandle_t missionQueue, errorSignalMailbox, missionStepReachedMailbox;

void RP_MainTask();
openMV_msg parse_openmv_msg(char * buf);
void send_openmv_nxt_state(Mission_states ms);
void RP_MissionTask();
void startNewMission();
void abortMission();
void callbackInterrupt(void *);
bool_t missionLogic(openMV_msg msg, bool_t * stepReached); //Ejecuta la lógica de recorrida de camino, y devuelve si terminó la misión

/*==================[internal functions definition]==========================*/


/*******Tasks*********/
void RP_MainTask()
{
	const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
	for( ;; )
	{
		if(queuesAttached) //Si no se adjuntaron las colas de comunicación no hace nada
		{
			xQueueReceive(missionQueue,&ms,portMAX_DELAY); //Espera a que llegue una misión
			if(ms.com== MISSION_START || ms.com==MISSION_REPLACE)
				startNewMission();
			else
				abortMission();
		}
		else
			vTaskDelay( xDelay250ms );
	}


}
void RP_MissionTask()
{
	openMV_msg msg;
	bool_t quit,stepReached;
	for( ;; )
	{
		xSemaphoreTake( xBinarySemaphore, portMAX_DELAY ); //Espera hasta que haya nuevos datos del openMV
		stepReached=0;
		msg=parse_openmv_msg(recBuff); //Se decodifica el msg
		quit=missionLogic(msg, &stepReached);
		xQueueSendToFront(errorSignalMailbox,&msg.displacement,0 ); //Envía el error
		xQueueSendToFront(missionStepReachedMailbox,&stepReached,0 ); //Avisa que se avanzó un paso en la misión
		if(quit)
			abortMission();
	}
}




/*******Otros*********/
bool_t missionLogic(openMV_msg msg, bool_t *stepReached)
{
	Mission_states currState = ms.md.currMission[ms.md.currStep];
	Mission_states nextState;
	bool_t missionFinished,stepsLeft=1;

	if(ms.md.currStep == ms.md.missionLen-1)
		stepsLeft = 0;
	else
		nextState= ms.md.currMission[ms.md.currStep+1];//Si quedan pasos próximos, obtengo el proximo paso de la misión

	bool_t steppingCondition = ( 	((currState == MISSION_SLOW_DOWN) && msg.tag_found && msg.tag==TAG_SLOW_DOWN) 	||
									((currState == MISSION_SPEED_UP) && msg.tag_found && msg.tag==TAG_SPEED_UP) 	||
									((currState == MISSION_STATION) && msg.tag_found && msg.tag==TAG_STATION) 		||
									(IS_FORKORMERGE_MISSION(currState) && msg.form_passed)
								); //Condiciones para ir al siguiente paso de la misión

	if (steppingCondition)
	{
		ms.md.currStep++;
		*stepReached=1;
		if(stepsLeft)
			send_openmv_nxt_state(nextState);
	}


	if(ms.md.currStep == ms.md.missionLen-1)
		missionFinished=1;
	return missionFinished;
}
void startNewMission()
{
	for(unsigned int i=0; i<OPEN_MV_MSG_LEN; i++ )
		uartReadByte(RP_UART,(uint8_t *) &(recBuff[i]));// Si quedó basura en la cola de la uart la vuela
	uartCallbackSet( RP_UART, UART_RECEIVE,(callBackFuncPtr_t) callbackInterrupt);
	xTaskCreate( RP_MissionTask, "RP mission task", 100	, NULL, 2, missionTask ); //Crea task de misión
	uartInterrupt( RP_UART, 1 ); //Enables uart interrupts
}
void abortMission()
{
	uartInterrupt( RP_UART, 0 ); //Disables interrupts
	uartTxWrite(RP_UART,OPENMV_IDLE);
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
void send_openmv_nxt_state(Mission_states ms)
{
	if(ms == MISSION_FORK_LEFT)
		uartTxWrite(RP_UART,OPENMV_FORK_LEFT);
	else if(ms == MISSION_FORK_RIGHT)
		uartTxWrite(RP_UART,OPENMV_FORK_RIGHT);
	else if(ms == MISSION_STATION || ms == MISSION_SLOW_DOWN || ms==MISSION_SPEED_UP)
		uartTxWrite(RP_UART,OPENMV_FOLLOW_LINE);
	else if(ms == MISSION_MERGE)
		uartTxWrite(RP_UART,OPENMV_MERGE);

}

void callbackInterrupt(void* a)
{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //Seccion 6.4 semáforos binarios freeRTOS
	for(unsigned int i=0; i<OPEN_MV_MSG_LEN; i++ )
		uartReadByte(RP_UART,(uint8_t *) &(recBuff[i]));// Read from RX FIFO
	xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); //Esto no lo entiendo bien
}
/*==================[external functions declaration]=========================*/

void RP_Init(void)
{
	uartInit( RP_UART, RP_UART_BAUDRATE, 1 );
	xBinarySemaphore = xSemaphoreCreateBinary();
	xTaskCreate( RP_MainTask, "RP Main task", 100	, NULL, 1, NULL ); //Crea task de misión
}

void RP_attachQueues(QueueHandle_t mission_queue,QueueHandle_t error_signal_mailbox,QueueHandle_t mission_step_reached_mailbox )
{
	missionQueue=mission_queue;
	errorSignalMailbox=error_signal_mailbox;
	missionStepReachedMailbox=mission_step_reached_mailbox;
	queuesAttached=1;
}
