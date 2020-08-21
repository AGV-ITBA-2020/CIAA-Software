/*
 * ProcDeCamino.c
 *
 *  Created on: 16 jun. 2020
 *      Author: mdevo
 */

#include "ProcDeCamino.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// Only for simulation
#include "timers.h"
#include "my_sapi.h"


typedef struct RawPathData{
	int32_t trackData;
	bool leftMarker;
	bool rightMarker;
}RawPathData;

void PC_MainTask(void *pvParams);
void ADC_StartConversion();
void ADC_ISR();

static void (*StartRequest)(void);	// Function to be called to start a new path measurement (track + marker)
QueueHandle_t queueData;
TimerHandle_t timerAdcFinished;

bool_t PC_Init()
{
	bool_t retVal = true;
	if(PC_INPUT_TYPE == PC_ADC)
	{
		// Config ADC
		StartRequest = ADC_StartConversion;
	}

	queueData = xQueueCreate(1, sizeof(RawPathData));
	if(queueData == NULL)
		retVal = false;
	else if(xTaskCreate(PC_MainTask, "PC_TASK", 512, NULL, PC_TASK_PRIO, NULL) == pdFALSE)
		retVal = false;

	// FOR SIMULATION:
	timerAdcFinished = xTimerCreate("ADC Timer", pdMS_TO_TICKS(10), pdFALSE, NULL, ADC_ISR);
	configASSERT(timerAdcFinished != NULL);
	return retVal;
}

void PC_MainTask(void *pvParams)
{
	TickType_t xLastTimeWoke;
	const TickType_t trackSamplingPeriod = pdMS_TO_TICKS(TRACK_SAMPLE_PERIOD);
	RawPathData pathData;

	xLastTimeWoke = xTaskGetTickCount();
	StartRequest();	// To accomodate first entry to task
	vTaskDelayUntil(&xLastTimeWoke, trackSamplingPeriod);	// Init data request and sleep until next period
	for( ;; )
	{
		BaseType_t okFlag = xQueueReceive(queueData, &pathData, trackSamplingPeriod/2);	// Wait half a sampling period MAX
		configASSERT(okFlag != errQUEUE_EMPTY); // Este assert debería ser para atrapar errores de configuracion y programación.
												// Este error deberia ser de hardware y se deberia utilizar otro tipo de assert...
		// Data is now stored on pathData variable
		printf("PC: Track=%d ; Markers=(%d,%d)\r\n", pathData.trackData, pathData.leftMarker, pathData.rightMarker);
		ADC_StartConversion();
		vTaskDelayUntil(&xLastTimeWoke, trackSamplingPeriod);
	}
}

void ADC_StartConversion()
{
	BaseType_t timerOk = xTimerStart(timerAdcFinished, 0);
	configASSERT(timerOk == pdPASS);
}

void ADC_ISR()
{
	gpioToggle(DO6);
	RawPathData data = {200, true, false};
	xQueueSendFromISR(queueData, &data, pdFALSE);
}
