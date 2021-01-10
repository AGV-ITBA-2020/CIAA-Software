/*
 * PathControlModule.c
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */


/*==================[inclusions]=============================================*/
#include "my_sapi_uart.h"

#include "HMIWrapper.hpp"
#include "PathControlProcess.h"
#include "MovementControlModule.hpp"
#include "GlobalEventGroup.h"
#include "PID_v1.hpp"
using namespace pid;

#include "event_groups.h"
#include "semphr.h"


/*==================[macros and definitions]=================================*/
#define OPEN_MV_MSG_LEN 4 //Length en bytes del mensaje del openMV
#define MAX_DISPLACEMENT 80.0
#define EVENT_QUEUE_LEN 10
#define AGV_CRUISE_SPEED 0.16	// Normal speed=0.16

#define LOW_SPEED_VEL 0.08
#define HIGH_SPEED_VEL 0.45
#define PCP_OPENMV_PROCESSING_PERIOD_MS 100
#define N_ERR_TO_STOP 30

#define MIN_DISTANCE_BETWEEN_TAGS 0.5
#define DIST_TO_STOP_SHOWING_LIGHTS_STATION 0.2
#define MIN_DISTANCE_BETWEEN_CHECKPOINTS MIN_DISTANCE_BETWEEN_TAGS

typedef enum {OPENMV_FOLLOW_LINE, OPENMV_FORK_LEFT, OPENMV_FORK_RIGHT, OPENMV_MERGE, OPENMV_ERROR,OPENMV_IDLE,OPENMV_SEND_DATA=10}openMV_states; //Los distintos estados del OpenMV
typedef enum {TAG_SLOW_DOWN, TAG_SPEED_UP, TAG_STATION=3}Tag_t; //Los distintos TAGs

#define IS_FORKORMERGE_MISSION(x) ((x)==CHECKPOINT_FORK_LEFT || (x) == CHECKPOINT_FORK_RIGHT || (x) == CHECKPOINT_MERGE )

typedef struct  {
  int8_t displacement;
  bool_t tag_found;
  bool_t form_passed; //Fork or Merge passed: Indica si se terminï¿½ de pasar por el fork o merge.
  bool_t error;
  Tag_t tag;
}openMV_msg; //La informacion que trae cada mensaje del openMV.


/*==================[internal data declaration]==============================*/
static char recBuff[OPEN_MV_MSG_LEN];//Acï¿½ se guarda la data en la interrupciï¿½n
extern EventGroupHandle_t xEventGroup;
static BLOCK_DETAILS_T * missionBlock;
static SemaphoreHandle_t xBinarySemaphore;
static TaskHandle_t * missionTaskHandle;
static TaskHandle_t * openMVSendTaskHandle;
static uint8_t msgCodeForOpenMV;
static unsigned int err_count;

typedef struct {
	double error;
	double output;
	double w_output;
	double v_output;
	double setpoint;
}AGV_SPEED_T;

AGV_SPEED_T agvSpeedData;

// static double PID_KP = 0.19;	// Tenttivas para v mas grandes
// static double PID_KI = 0;
// static double PID_KD = 0.15;

static double PID_KP = 0.14;	// Para velocidades entre 0.25 y 0.15 m/s
static double PID_KI = 0;
static double PID_KD = 0.11;
static PID pidController(&(agvSpeedData.error), &(agvSpeedData.output), &(agvSpeedData.setpoint), PID_KP, PID_KI, PID_KD, DIRECT);

/*==================[internal functions declaration]=========================*/
openMV_msg parseOpenMVMsg(char * buf);
void setOpenMVNextState(BLOCK_CHECKPOINT_T ms);
void missionTask(void * ptr);
void callbackInterrupt(void *);
bool_t missionBlockLogic(openMV_msg msg, bool_t * stepReached); //Ejecuta la lï¿½gica de recorrida de camino, y devuelve si terminï¿½ la misiï¿½n
static void computeAngVel(int displacement); //Obtiene la velocidad angular objetivo (Acï¿½ deberï¿½a estar el PID).
void openMVSendTask(void * ptr);
void deleteMovTasks();
/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/*******Tasks*********/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
double PCP_GetPIDError()
{
	return agvSpeedData.error;
}
void missionTask(void * ptr)
{
	openMV_msg msg;
	bool_t quit,stepReached;
	for( ;; )
	{
		xSemaphoreTake( xBinarySemaphore, portMAX_DELAY ); //Espera hasta que haya nuevos datos del openMV
		stepReached=0;
		msg=parseOpenMVMsg(recBuff); //Se decodifica el msg
		quit=missionBlockLogic(msg, &stepReached);//Se aplica las lï¿½gicas de camino, determinando si se llegï¿½ al paso de misiï¿½n y si se terminï¿½ la misiï¿½n
		
		computeAngVel(msg.displacement);
		if(MC_GetManualMode() == false)	// Set AGV speeds only if manual mode is off
		{
			MC_setLinearSpeed(agvSpeedData.v_output); 	//Se setean las velocidades para el seguimiento de lï¿½nea
			MC_setAngularSpeed(agvSpeedData.v_output > 0 ? agvSpeedData.w_output : 0.0);
		}


		if(stepReached) //Levanto los eventos correspondientes
		{
			if(quit)
			{
				xEventGroupSetBits( xEventGroup, GEG_CTMOVE_FINISH | GEG_MISSION_STEP_REACHED);
				PCP_abortMissionBlock(); //Si finaliza la misiï¿½n, termina el proceso de misiï¿½n
			}else{
				xEventGroupSetBits( xEventGroup, GEG_MISSION_STEP_REACHED );
			}
		}
	}
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void openMVSendTask(void * ptr)
{
	const TickType_t xDelay50ms = pdMS_TO_TICKS( PCP_OPENMV_PROCESSING_PERIOD_MS );
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		uartTxWrite(PC_UART,msgCodeForOpenMV);//Cada 50ms envï¿½a el cï¿½digo correspondiente al openMV // @suppress("Invalid arguments")
		vTaskDelayUntil( &xLastWakeTime, xDelay50ms ); // @suppress("Invalid arguments")
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
	BLOCK_CHECKPOINT_T currChkpnt = missionBlock->blockCheckpoints[missionBlock->currStep];
	BLOCK_CHECKPOINT_T nextChkpnt;
	bool_t missionFinished=0,stepsLeft=1;

	if(missionBlock->currStep == (missionBlock->blockLen-1)) //Para saber si estoy en el ï¿½ltimo bloque
		stepsLeft = 0;
	else
		nextChkpnt= missionBlock->blockCheckpoints[(missionBlock->currStep)+1];//Si quedan pasos proximos, obtengo el proximo paso de la mision

	bool_t steppingCondition = ( 	((currChkpnt == CHECKPOINT_SLOW_DOWN) && msg.tag_found && msg.tag==TAG_SLOW_DOWN) 	||
									((currChkpnt == CHECKPOINT_SPEED_UP) && msg.tag_found && msg.tag==TAG_SPEED_UP) 	||
									((currChkpnt == CHECKPOINT_STATION) && msg.tag_found && (msg.tag==TAG_STATION) && (MC_getDistanceTravelled() >= MIN_DISTANCE_BETWEEN_TAGS)) 		||
									(IS_FORKORMERGE_MISSION(currChkpnt) && msg.form_passed)
								); //Condiciones para ir al siguiente paso de la mision
	//Control de pasos de misiï¿½n
	if(msg.error==0)
		err_count=0;
	else
		err_count++;
	/*Manejo de luces para luego de estación */
	if(HMI_IsOutputSet(OUTPUT_LEDSTRIP_LEFT) && MC_getDistanceTravelled()>DIST_TO_STOP_SHOWING_LIGHTS_STATION)
		HMI_ClearOutput(OUTPUT_LEDSTRIP_LEFT);
	if(HMI_IsOutputSet(OUTPUT_LEDSTRIP_RIGHT) && MC_getDistanceTravelled()>DIST_TO_STOP_SHOWING_LIGHTS_STATION)
		HMI_ClearOutput(OUTPUT_LEDSTRIP_RIGHT);

	/*Avance de misiones*/
	if (steppingCondition && MC_getDistanceTravelled()>MIN_DISTANCE_BETWEEN_CHECKPOINTS )
	{
		HMI_ClearOutput(OUTPUT_LEDSTRIP_LEFT); //Siempre que avanza un paso de la misión, borra todo lo previo
		HMI_ClearOutput(OUTPUT_LEDSTRIP_RIGHT);//Que se estaba haciendo con los guiños.
		(missionBlock->currStep)++;
		*stepReached=true; //Indico que se llegï¿½ a un paso
		switch (currChkpnt) 
		{
			case CHECKPOINT_FORK_LEFT:
				HMIW_Blink(OUTPUT_LEDSTRIP_LEFT,5);
			break;
			case CHECKPOINT_FORK_RIGHT:
				HMIW_Blink(OUTPUT_LEDSTRIP_RIGHT,5);
			break;
			case CHECKPOINT_MERGE:
				HMIW_Blink(OUTPUT_LEDSTRIP_LEFT,5);
				HMIW_Blink(OUTPUT_LEDSTRIP_RIGHT,5);
			break;
			case CHECKPOINT_STATION:
				HMIW_SetOutput(OUTPUT_LEDSTRIP_LEFT, true);
				HMIW_SetOutput(OUTPUT_LEDSTRIP_RIGHT, true);
			break;
		}

		if(stepsLeft)
			setOpenMVNextState(nextChkpnt); //En el caso que no quedan misiones, guardo el prï¿½ximo mensaje que se le va a enviar al openMV
	}
	else
		msgCodeForOpenMV=OPENMV_SEND_DATA; //Guardo que el prï¿½ximo dato que se le envï¿½e al openmv es que siga tirando datos.
	//Control de velocidad
	if ((currChkpnt == CHECKPOINT_SLOW_DOWN) && msg.tag_found && msg.tag==TAG_SLOW_DOWN)
		agvSpeedData.v_output =LOW_SPEED_VEL;
	if ((currChkpnt == CHECKPOINT_SPEED_UP) && msg.tag_found && msg.tag==TAG_SPEED_UP)
		agvSpeedData.v_output =HIGH_SPEED_VEL;
	if(missionBlock->currStep == missionBlock->blockLen)//Si ahora se llegï¿½ al final de la misiï¿½n, quit=1
		missionFinished=1;

	if(err_count >= N_ERR_TO_STOP)
		xEventGroupSetBits( xEventGroup, GEG_CTMOVE_ERROR);
	return missionFinished;
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
openMV_msg parseOpenMVMsg(char * buf)
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
void setOpenMVNextState(BLOCK_CHECKPOINT_T ms)
{ //Correlaciï¿½n entre los estados del openmv y el checkpoint que viene.
	if(ms == CHECKPOINT_FORK_LEFT)
		msgCodeForOpenMV=OPENMV_FORK_LEFT;
	else if(ms == CHECKPOINT_FORK_RIGHT)
		msgCodeForOpenMV=OPENMV_FORK_RIGHT;
	else if(ms == CHECKPOINT_STATION || ms == CHECKPOINT_SLOW_DOWN || ms==CHECKPOINT_SPEED_UP)
		msgCodeForOpenMV=OPENMV_FOLLOW_LINE;
	else if(ms == CHECKPOINT_MERGE)
		msgCodeForOpenMV=OPENMV_MERGE;
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void callbackInterrupt(void* a)
{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //Seccion 6.4 semï¿½foros binarios freeRTOS
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
static void computeAngVel(int displacement) //Desarrollar con PID
{
	agvSpeedData.error=(double)displacement / MAX_DISPLACEMENT; //Desplazamiento de la lï¿½nea entre -1 y 1.
	pidController.Compute();
	agvSpeedData.w_output = agvSpeedData.output * AGV_MAX_ANGULAR_SPEED;	// Map pid output to angular speed of agv
}
#define PID_OUTPUT_MAX (1)
#define PID_OUTPUT_MIN (-1)
void deleteMovTasks()
{
	if(missionTaskHandle!=NULL)
		vTaskDelete(missionTaskHandle); //Borra la task de misiï¿½n
	if(openMVSendTaskHandle!=NULL)
		vTaskDelete(openMVSendTaskHandle); //Borra la task de misiï¿½n
}
/*==================[external functions definition]==========================*/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_Init(void){
	xBinarySemaphore = xSemaphoreCreateBinary();
	msgCodeForOpenMV=OPENMV_IDLE;
	uartInit( PC_UART, 115200, 0 );
	uartCallbackSet( PC_UART, UART_RECEIVE,(callBackFuncPtr_t) callbackInterrupt);
	missionTaskHandle =NULL;
	openMVSendTaskHandle= NULL;
	err_count=0;
	MC_Init();
	// Turn the PID on
  	pidController.SetMode(AUTOMATIC);
  	pidController.SetOutputLimits(PID_OUTPUT_MIN, PID_OUTPUT_MAX);
	pidController.SetSampleTime(PCP_OPENMV_PROCESSING_PERIOD_MS);
	agvSpeedData.setpoint = 0;	// Queremos que el error con la linea sea 0 siempre.
}
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_startNewMissionBlock(BLOCK_DETAILS_T * mb)
{
	missionBlock = mb;
	agvSpeedData.v_output = AGV_CRUISE_SPEED;
	//MC_setDistanceTravelled(0); //NO! puedo haber abortado a la mitad del camino y en dist travelled se guarda esa info
	setOpenMVNextState(missionBlock->blockCheckpoints[0]);
	PCP_continueMissionBlock();
	//HMIW_SetOutput(OUTPUT_LEDSTRIP_LEFT, false);
	//HMIW_SetOutput(OUTPUT_LEDSTRIP_RIGHT, false);
}
void PCP_SetLinearSpeed(double v)
{
	agvSpeedData.v_output = v;
}
double PCP_getLinearSpeed()
{
	return agvSpeedData.v_output ;
}
void PCP_setPIDTunings(double Kp, double Ki, double Kd)
{
	pidController.SetTunings(Kp, Ki, Kd, 1);
}
void PCP_getPIDTunings(double* Kp, double* Ki, double* Kd)
{
	*Kp = pidController.GetKp();
	*Ki = pidController.GetKi();
	*Kd = pidController.GetKd();
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_abortMissionBlock(void)
{
	PCP_pauseMissionBlock();
	uartTxWrite(PC_UART,OPENMV_IDLE);
}
void PCP_pauseMissionBlock(void)
{
	uartInterrupt( PC_UART, 0 ); //Disables interrupts
	deleteMovTasks();
	//currVel=0;
#ifndef DEBUG_WITHOUT_MC
	MC_setLinearSpeed(0);
	MC_setAngularSpeed(0);
#endif

}

void PCP_continueMissionBlock(void)
{
	while(uartReadByte(PC_UART,(uint8_t *) &(recBuff[0])));// Si quedï¿½ basura en la cola de la uart la vuela
	xTaskCreate( missionTask, "PCP mission task", 200	, NULL, 2, missionTaskHandle ); //Crea task de misiï¿½n
	xTaskCreate( openMVSendTask, "Send to openMV", 100	, NULL, 3, openMVSendTaskHandle );
	err_count=0;
	uartInterrupt( PC_UART, 1 ); //Enables uart interrupts
}
