/*
 * HMI.c
 *
 *  Created on: 20 ago. 2020
 *      Author: mdevo
 */

#include "HMI.h"
#include "my_sapi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#define HMI_TASK_PRIO 0
#define HMI_IO_ID_MAX_BIT_SIZE 6
#define HMI_QUEUE_LENGTH 5

#define HMI_REFRESH_LOW pdMS_TO_TICKS(100)	// ms. Low frequency refresh rate
#define HMI_REFRESH_HIGH pdMS_TO_TICKS(10)	// ms. High frequency refresh rate

typedef struct {
	bool IO_type;		// 0 indicates an input. 1 indicates an output.
	HMI_INPUT_ID id : HMI_IO_ID_MAX_BIT_SIZE;
	HMI_INPUT_PATTERN pattern : 3;				// Indicates the type of pattern to capture.
	unsigned int maxCount : 3;					// Indicates the number of times the pattern repeats for a success. If 0, input is not active. Para short press es el número de short press necesarios para cumplir. Para long press es el tiempo máximo para ser considerado long press.
	unsigned int count : 3;						// Used to count patterns. Para short press cuento cuántos voy teniendo. Para long press lo uso para el tiempo que permanece presionado.
	bool lastValue : 1;							// Used for capturing input changes and debouncing.
	gpioMap_t inputPin;
	void (* callbackSuccess)(HMI_INPUT_ID inputId);
	void (* callbackAbort)(HMI_INPUT_ID inputId);
}HMI_Input_t;

typedef struct {
	bool IO_type;		// 0 indicates an input. 1 indicates an output.
	gpioMap_t outputPin;
	HMI_OUTPUT_ID id;
	unsigned int timeOn;						// Indicates the # of timebases to set the output ON in one period. If 0, input is not active.
	unsigned int timeOff;					// Indicates the # of timebases to set the output OFF in one period.
	unsigned int timebaseCounter;			// Counts the timebase in each period.
	unsigned int actionCounter;			// Indicates the amount of periods to execute the pattern.
	void (* callbackSuccess)(HMI_OUTPUT_ID inputId);
	void (* callbackAbort)(HMI_OUTPUT_ID inputId);
}HMI_Output_t;

static HMI_Input_t inputArray[INPUT_TOTAL_COUNT];
static HMI_Output_t outputArray[OUTPUT_TOTAL_COUNT];
static QueueHandle_t hmiQueue;
static char queueObjectSize;	// Used to define max size of object in queue

void HMI_MainTask();
char RunInputRoutine();
char RunOutputRoutine();
void LoadInputConfig(HMI_Input_t * data);
void LoadOutputConfig(HMI_Output_t * data);

void HMI_Init()
{
	// Initialize HMI task
	BaseType_t taskOk = xTaskCreate(HMI_MainTask, "HMI_TASK", 512, NULL, HMI_TASK_PRIO, NULL);
	configASSERT(taskOk == true);

	// Initialize queue
	queueObjectSize = sizeof(HMI_Output_t);
	if(sizeof(HMI_Input_t) > queueObjectSize)
		queueObjectSize = sizeof(HMI_Input_t);
	hmiQueue = xQueueCreate(HMI_QUEUE_LENGTH, (UBaseType_t) queueObjectSize);
	configASSERT(hmiQueue != NULL);
}

void HMI_MainTask()
{
	// Set initial variables
	TickType_t xLastTimeWoke = xTaskGetTickCount();
	TickType_t hmiUpdatePeriod = HMI_REFRESH_HIGH;

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

		char nInputs = RunInputRoutine();
		char nOutputs = RunOutputRoutine();
		if(nInputs > 0) // If there's at least 1 input active
			hmiUpdatePeriod = HMI_REFRESH_HIGH;
		else if(nOutputs > 0) // If there's at no input but at least 1 output active
			hmiUpdatePeriod = HMI_REFRESH_LOW;
		else	// Else no IO active. Sleep until new queue event.
			hmiUpdatePeriod = portMAX_DELAY;

		if(hmiUpdatePeriod == portMAX_DELAY)  // No IO active. Sleep until new queue event.
			xQueuePeek(hmiQueue, NULL, portMAX_DELAY);
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
	if(inputArray[i].maxCount==0)
		continue;
	else
	{
		if(gpioRead(inputArray[i].inputPin)==0)
		{
			if(inputArray[i].lastValue==1)
			{
				if(inputArray[i].pattern==LONG_PRESS)
				{
					(inputArray[i].count)=0;
					inputArray[i].lastValue=gpioRead(inputArray[i].inputPin);
					ninputs++;
				}

				else
				{
					(inputArray[i].count)++;

					if(inputArray[i].count<inputArray[i].maxCount)
					{
						inputArray[i].lastValue=gpioRead(inputArray[i].inputPin);
						ninputs++;
					}
					else
					{
						inputArray[i].maxCount=0;
						inputArray[i].callbackSuccess;
					}
				}
			}
			else
			{
				if(inputArray[i].pattern==LONG_PRESS)
				{
					(inputArray[i].count)++;

					if(inputArray[i].count<inputArray[i].maxCount)
					{
						inputArray[i].lastValue=gpioRead(inputArray[i].inputPin);
						ninputs++;
					}
					else
					{
						inputArray[i].maxCount=0;
						inputArray[i].callbackSuccess;
					}

				}
				else

					inputArray[i].callbackAbort;
			}
		}
		else
			if(inputArray[i].lastValue==0)
			{

			}


	}
	}

	return ninputs;


}

char RunOutputRoutine()
{
	unsigned int noutputs;

	for(int i=0;i<OUTPUT_TOTAL_COUNT;i++)
		if((outputArray[i].timebaseCounter)<(outputArray[i].timeOn))
		{
			(outputArray[i].timebaseCounter)++;
			gpioWrite(outputArray[i].outputPin,1);
		}

		else if((outputArray[i].timebaseCounter>=outputArray[i].timeOn) && (outputArray[i].timebaseCounter<=((outputArray[i].timeOff) + (outputArray[i].timeOff))))
		{
			(outputArray[i].timebaseCounter)++;
			gpioWrite(outputArray[i].outputPin,0);
		}
	
		else
		{
			(outputArray[i].timebaseCounter)=0;
			outputArray[i].callbackSuccess;
		}

	return noutputs;
}

void LoadInputConfig(HMI_Input_t * data)
{
	inputArray[data.id]=data;

}

void LoadOutputConfig(HMI_Output_t * data)
{

	outputArray[data.id]=data;

}
