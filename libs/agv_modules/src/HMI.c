/*
 * HMI.c
 *
 *  Created on: 20 ago. 2020
 *      Author: mdevo
 */

#include "HMI.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#define HMI_TASK_PRIO 0
#define HMI_IO_ID_MAX_BIT_SIZE 6
#define HMI_QUEUE_LENGTH 5

#define HMI_REFRESH_RATE pdMS_TO_TICKS(HMI_REFRESH_MS)	// ms. High frequency refresh rate



static HMI_Input_t inputArray[INPUT_TOTAL_COUNT];
static HMI_Output_t outputArray[OUTPUT_TOTAL_COUNT];
static QueueHandle_t hmiQueue;
static char queueObjectSize;	// Used to define max size of object in queue

static gpioMap_t inputConnectionMap[]={
		DI2, 	//INPUT_BUT_GREEN
		DI3, 	//INPUT_BUT_BLUE
		DI0, 	//INPUT_SW_AUTO
		DI1 	//INPUT_SW_MANUAL
};
static gpioMap_t outputConnectionMap[]={
		DO5, 	//OUTPUT_BUT_GREEN
		DO6, 	//OUTPUT_BUT_BLUE
		DO0, 	//OUTPUT_LEDSTRIP_RIGHT
		DO1, 	//OUTPUT_LEDSTRIP_LEFT
		DO2, 	//OUTPUT_LEDSTRIP_STOP
		DO4 	//OUTPUT_BUZZER
};


void HMI_MainTask();
char RunInputRoutine();
char RunOutputRoutine();
void LoadInputConfig(HMI_Input_t * data);
void LoadOutputConfig(HMI_Output_t * data);
void GpioInit();


////////////////////////////Funciones externas ///////////////////////////////////
void HMI_Init()
{
	// Initialize HMI task
	BaseType_t taskOk = xTaskCreate(HMI_MainTask, "HMI_TASK", 256, NULL, HMI_TASK_PRIO, NULL);
	// Initialize queue
	GpioInit();
	queueObjectSize = sizeof(HMI_Output_t);
	if(sizeof(HMI_Input_t) > queueObjectSize)
		queueObjectSize = sizeof(HMI_Input_t);
	hmiQueue = xQueueCreate(HMI_QUEUE_LENGTH, (UBaseType_t) queueObjectSize);
}
bool_t HMI_AddToQueue(void * HMI_InputOrOutput)
{
	return xQueueSendToBack(hmiQueue,HMI_InputOrOutput,0 );
}
gpioMap_t HMI_getCorrespondingPin(HMI_IO_TYPE IOType, unsigned int id)
{
	gpioMap_t retVal;
	if(IOType == HMI_INPUT)
		retVal=inputConnectionMap[id];
	else
		retVal=outputConnectionMap[id];
	return retVal;
}

void HMI_SetOutputPin(gpioMap_t pin, bool_t state)
{
	gpioWrite(pin,state);
}

////////////////////////////Funciones internas ///////////////////////////////////
void HMI_MainTask()
{
	// Set initial variables
	TickType_t xLastTimeWoke = xTaskGetTickCount();
	TickType_t hmiUpdatePeriod = HMI_REFRESH_RATE;

	// Start task main loop
	for( ;; )
	{
		// First, load all pending IO request in the corresponding IO struct
		while(uxQueueMessagesWaiting(hmiQueue) > 0)
		{
			char tempObj[queueObjectSize];
			xQueueReceive(hmiQueue, tempObj, 0);
			if(*tempObj == 0)	// Object is an input
				LoadInputConfig((HMI_Input_t *) tempObj);
			else
				LoadOutputConfig((HMI_Output_t *) tempObj);
		}

		/*
		HMI_Input_t tempObj;
		tempObj.IO_type=0;
		tempObj.count=0;
		tempObj.id=0;
		tempObj.inputPin;
		tempObj.lastValue=1;
		tempObj.maxCount=500;
		tempObj.patCount=1;
		tempObj.pattern=COUNTER;
		HMI_Input_t *pointer =tempObj;

		LoadInputConfig((HMI_Input_t *) pointer);
		*/

		char nInputs = RunInputRoutine();
		char nOutputs = RunOutputRoutine();
		if(nInputs > 0 || nOutputs > 0) // If there's at least 1 input active
			hmiUpdatePeriod = HMI_REFRESH_RATE;
		else	// Else no IO active. Sleep until new queue event.
			hmiUpdatePeriod = portMAX_DELAY;

		if(hmiUpdatePeriod == portMAX_DELAY)  // No IO active. Sleep until new queue event.
		{
			char tempObj[queueObjectSize];
			xQueuePeek(hmiQueue, tempObj, portMAX_DELAY);
		}
		else
		{
			vTaskDelayUntil(&xLastTimeWoke, hmiUpdatePeriod);
		}
	}
}

char RunInputRoutine()
{
	unsigned int ninputs=0;
	for(int i=0;i<INPUT_TOTAL_COUNT;i++)
	{
		if(inputArray[i].maxCount==0)								// Checks if input is active
			continue;
		else
		{
			int pinread=gpioRead(inputArray[i].inputPin);
			ninputs++;
			if(pinread==0)
			{
				if(inputArray[i].lastValue==1)						// If a 0 has been read and the last value was 1 -> reset counter, update lastValue and set the input as active.
					(inputArray[i].count)=0;
				else												// If a 0 has been read and the last value was 0 -> update time pressed
					(inputArray[i].count)++;					// Whether needed time is reached or not -> update last value, set input as active. // Whether needed time is reached or not -> do nothing because the button is still being pressed.
			}
			else if(inputArray[i].lastValue==0)					// If a 1 has been read and the last value was 0 -> Button released
			{
				if(inputArray[i].count>inputArray[i].maxCount)	// If needed time is reached
				{
					(inputArray[i].patCount--);
					if(inputArray[i].patCount==0)				// If number of patterns reached -> set input as inactive, successful press.
					{
						inputArray[i].maxCount=0;
						inputArray[i].callbackSuccess(inputArray[i].id);
					}
				}
			}
			inputArray[i].lastValue=pinread;
		}
	}
	return ninputs;
}

char RunOutputRoutine()
{
	unsigned int noutputs=0;

	for(int i=0;i<OUTPUT_TOTAL_COUNT;i++)
		if(outputArray[i].actionCounter==0)												// Checks if output is active
			continue;
		else
		{
			noutputs++;
			if((outputArray[i].timebaseCounter)<(outputArray[i].timeOn))				// If needed ON time is reached ->
			{
				(outputArray[i].timebaseCounter)++;
				gpioWrite(outputArray[i].outputPin,1);
			}
			else if((outputArray[i].timebaseCounter>=outputArray[i].timeOn) && (outputArray[i].timebaseCounter<=((outputArray[i].timeOn) + (outputArray[i].timeOff))))
			{
				(outputArray[i].timebaseCounter)++;
				gpioWrite(outputArray[i].outputPin,0);
			}

			else
			{
				(outputArray[i].actionCounter)--;
				(outputArray[i].timebaseCounter)=0;
				if(outputArray[i].actionCounter==0)
					outputArray[i].callbackSuccess(outputArray[i].id);
			}
	
		}

	return noutputs;
}

void LoadInputConfig(HMI_Input_t * data)
{
	if(inputArray[data->id].maxCount==0)
		inputArray[data->id]=*data;
	else
		data->callbackAbort(data->id);

}

void LoadOutputConfig(HMI_Output_t * data)
{
	if(outputArray[data->id].actionCounter==0)
		outputArray[data->id]=*data;
	/*
	else
		data->callbackAbort(data->id);*/
}
void GpioInit()
{
	for(unsigned int i=0; i<INPUT_TOTAL_COUNT;i++)
		gpioInit(inputConnectionMap[i],GPIO_INPUT);
	for(unsigned int i=0; i<OUTPUT_TOTAL_COUNT;i++)
		gpioInit(outputConnectionMap[i],GPIO_OUTPUT);
}

