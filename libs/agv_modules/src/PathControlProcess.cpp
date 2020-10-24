/*
 * PathControlModule.c
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */


/*==================[inclusions]=============================================*/
#include "my_sapi_uart.h"

#include "PathControlProcess.h"
#include "MovementControlModule.hpp"
#include "GlobalEventGroup.h"

#include "event_groups.h"
#include "semphr.h"


/*==================[macros and definitions]=================================*/
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
}openMV_msg; //La informaci�n que trae cada mensaje del openMV.


/*==================[internal data declaration]==============================*/
static char recBuff[OPEN_MV_MSG_LEN];//Ac� se guarda la data en la interrupci�n
extern EventGroupHandle_t xEventGroup;
static MISSION_BLOCK_T * missionBlock;
static SemaphoreHandle_t xBinarySemaphore;
static TaskHandle_t * missionTaskHandle;
static double currVel=0;

/*==================[internal functions declaration]=========================*/
openMV_msg parse_openmv_msg(char * buf);
void send_openmv_nxt_state(BLOCK_CHECKPOINT_T ms);
void missionTask(void * ptr);
void callbackInterrupt(void *);
bool_t missionBlockLogic(openMV_msg msg, bool_t * stepReached); //Ejecuta la l�gica de recorrida de camino, y devuelve si termin� la misi�n
double computeAngVel(unsigned int displacement); //Obtiene la velocidad angular objetivo (Ac� deber�a estar el PID).

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/*******Tasks*********/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void missionTask(void * ptr)
{
	openMV_msg msg;
	bool_t quit,stepReached;
	for( ;; )
	{
		xSemaphoreTake( xBinarySemaphore, portMAX_DELAY ); //Espera hasta que haya nuevos datos del openMV
		stepReached=0;
		msg=parse_openmv_msg(recBuff); //Se decodifica el msg
		quit=missionBlockLogic(msg, &stepReached);
		MC_setLinearSpeed(currVel);
		MC_setAngularSpeed(computeAngVel(msg.displacement));
		if(stepReached)
		{
			xEventGroupSetBits( xEventGroup, GEG_MISSION_STEP_REACHED );
		}
		if(quit)
		{
			xEventGroupSetBits( xEventGroup, GEG_MISSION_STEP_REACHED || GEG_CTMOVE_FINISH);
			PCP_abortMissionBlock();
		}
	}
}

/*******Otros*********/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
bool_t missionBlockLogic(openMV_msg msg, bool_t *stepReached)
{
	BLOCK_CHECKPOINT_T currChkpnt = missionBlock->md.blockCheckpoints[missionBlock->md.currStep];
	BLOCK_CHECKPOINT_T nextChkpnt;
	bool_t missionFinished,stepsLeft=1;

	if(missionBlock->md.currStep == missionBlock->md.blockLen-1)
		stepsLeft = 0;
	else
		nextChkpnt= missionBlock->md.blockCheckpoints[missionBlock->md.currStep+1];//Si quedan pasos pr�ximos, obtengo el proximo paso de la misi�n

	bool_t steppingCondition = ( 	((currChkpnt == CHECKPOINT_SLOW_DOWN) && msg.tag_found && msg.tag==TAG_SLOW_DOWN) 	||
									((currChkpnt == CHECKPOINT_SPEED_UP) && msg.tag_found && msg.tag==TAG_SPEED_UP) 	||
									((currChkpnt == CHECKPOINT_STATION) && msg.tag_found && msg.tag==TAG_STATION) 		||
									(IS_FORKORMERGE_MISSION(currChkpnt) && msg.form_passed)
								); //Condiciones para ir al siguiente paso de la misi�n
	//Control de pasos de misi�n
	if (steppingCondition)
	{
		missionBlock->md.currStep++;
		*stepReached=true;
		if(stepsLeft)
			send_openmv_nxt_state(nextChkpnt);
	}
	//Control de velocidad
	if ((currChkpnt == CHECKPOINT_SLOW_DOWN) && msg.tag_found && msg.tag==TAG_SLOW_DOWN)
		currVel=LOW_SPEED_VEL;
	if ((currChkpnt == CHECKPOINT_SPEED_UP) && msg.tag_found && msg.tag==TAG_SPEED_UP)
		currVel=HIGH_SPEED_VEL;
	if(missionBlock->md.currStep == missionBlock->md.blockLen-1)
		missionFinished=1;
	return missionFinished;
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
openMV_msg parse_openmv_msg(char * buf)
{
	openMV_msg retVal;
	retVal.displacement=buf[0];
	retVal.error= (buf[1] /128)%2;
	retVal.form_passed= (buf[1]/64)%2;
	retVal.tag_found= (buf[1]/32)%2;
	retVal.tag=(Tag_t)((unsigned int)buf[1]%32);
	return retVal;
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void send_openmv_nxt_state(BLOCK_CHECKPOINT_T ms)
{ //Correlaci�n entre los estados del openmv y el checkpoint que viene.
	if(ms == CHECKPOINT_FORK_LEFT)
		uartTxWrite(PC_UART,OPENMV_FORK_LEFT);
	else if(ms == CHECKPOINT_FORK_RIGHT)
		uartTxWrite(PC_UART,OPENMV_FORK_RIGHT);
	else if(ms == CHECKPOINT_STATION || ms == CHECKPOINT_SLOW_DOWN || ms==CHECKPOINT_SPEED_UP)
		uartTxWrite(PC_UART,OPENMV_FOLLOW_LINE);
	else if(ms == CHECKPOINT_MERGE)
		uartTxWrite(PC_UART,OPENMV_MERGE);
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void callbackInterrupt(void* a)
{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //Seccion 6.4 sem�foros binarios freeRTOS
	for(unsigned int i=0; i<OPEN_MV_MSG_LEN; i++ )
		uartReadByte(PC_UART,(uint8_t *) &(recBuff[i]));// Read from RX FIFO
	xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); //Esto no lo entiendo bien
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
double computeAngVel(unsigned int displacement) //Desarrollar con PID
{
	double a;
	return a;
}

/*==================[external functions definition]==========================*/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_Init(void){
	xBinarySemaphore = xSemaphoreCreateBinary();
}
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_startNewMissionBlock(MISSION_BLOCK_T * mb)
{
	missionBlock = mb;
	currVel=HIGH_SPEED_VEL;
	for(unsigned int i=0; i<OPEN_MV_MSG_LEN; i++ )
		uartReadByte(PC_UART,(uint8_t *) &(recBuff[i]));// Si qued� basura en la cola de la uart la vuela
	uartCallbackSet( PC_UART, UART_RECEIVE,(callBackFuncPtr_t) callbackInterrupt);
	xTaskCreate( missionTask, "RP mission task", 100	, NULL, 2, missionTaskHandle ); //Crea task de misi�n
	uartInterrupt( PC_UART, 1 ); //Enables uart interrupts
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_abortMissionBlock(void)
{
	currVel=0;
	MC_setLinearSpeed(currVel);
	uartInterrupt( PC_UART, 0 ); //Disables interrupts
	uartTxWrite(PC_UART,OPENMV_IDLE);
	if(missionTaskHandle!=NULL)
		vTaskDelete(missionTaskHandle); //Borra la task de misi�n

}