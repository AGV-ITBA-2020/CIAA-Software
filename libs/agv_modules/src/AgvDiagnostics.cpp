/*
 * AgvDiagnostics.c
 *
 *  Created on: 12 oct. 2020
 *      Author: mdevo
 */

#include "AgvDiagnostics.hpp"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include <stdio.h>
#include "DiagMessage.h"
#include "PathControlProcess.h"
#include "MovementControlModule.hpp"
#include "my_sapi_uart.h"
#include "task.h"
#include "timers.h"

using namespace std;

#define RX_BUFFER_SIZE 128
#define TICK_TIMER_BASE pdMS_TO_TICKS(200)
#define JOYSTICK_DEFAULT_TICK_TIMEOUT 10
#define PID_DEFAULT_TICK_PERIOD 2
#define RTOSDIAG_DEFAULT_TICK_PERIOD 50

#if(configGENERATE_RUN_TIME_STATS)	// If stats are saved, create array for storing stat text
		static char cStringBuffer[ 512 ];
#endif

#define TO_PRINT(x) ((int)(x*100.0))

typedef struct {
	bool on;
	uint8_t tickPeriod;	// Period of process execution
	uint8_t currTick;		// Number of ticks since last execution
} PERIODIC_SERVICE_T;

typedef struct {
	bool diagOn;
	PERIODIC_SERVICE_T pidViewer;
	bool pidViewer_sendTrack;
	PERIODIC_SERVICE_T joystick;
	PERIODIC_SERVICE_T rtosDiag;
} DIAG_STATE_T;


TaskHandle_t xMainTaskToNotify;
TimerHandle_t xDiagTimerHandle;
uint32_t rxIndex;
uint8_t rxBuffer[RX_BUFFER_SIZE];
DIAG_STATE_T info = { false, {false, PID_DEFAULT_TICK_PERIOD}, true, {false, JOYSTICK_DEFAULT_TICK_TIMEOUT}, {false, RTOSDIAG_DEFAULT_TICK_PERIOD} };
DiagMessage * msg;
bool timerIsOn = false;

static void MainTask(void *pvParameters);
static void xTimerCallbackFunc( TimerHandle_t xTimer );
static void uartRxCallback(void);
static void RunModuleServices();
static bool ProcessMessage();
static void SendSpeedValues();

void AgvDiag_Init()
{
	uartInit(UART_USB, 115200, false);
	uartInterrupt(UART_USB, true);
	uartCallbackSet(UART_USB, UART_RECEIVE, (callBackFuncPtr_t)uartRxCallback);

	configASSERT(xTaskCreate(MainTask, "AGV_DIAG_TASK", 180, NULL, 1, &xMainTaskToNotify) == pdTRUE);

	xDiagTimerHandle = xTimerCreate("DiagTimer", TICK_TIMER_BASE, pdTRUE, 0, xTimerCallbackFunc);
	configASSERT(xDiagTimerHandle != NULL);

	info.diagOn = true;
	info.pidViewer.tickPeriod = PID_DEFAULT_TICK_PERIOD;

//	info.pidViewer.on = true;
//	configASSERT(xTimerReset(xDiagTimerHandle, 0) == pdPASS);
}

static void xTimerCallbackFunc( TimerHandle_t xTimer )
{
	configASSERT( xTaskNotify(xMainTaskToNotify, 0, eSetValueWithOverwrite) == pdPASS);
}

static void MainTask(void *pvParameters)
{
	uint32_t rxSize = 0;	// If 0 -> Notification from timer; else message was received
	TimerHandle_t tickTimer;

	for( ;; )
	{
		if( xTaskNotifyWait(0, 0, &rxSize, portMAX_DELAY) == pdPASS )	// Then an event has been received
		{
			if(rxSize == 0)	// Notification from timer
			{
				RunModuleServices();
			}
			else 	// Notification from uart ISR
			{
				ProcessMessage();
			}
		}

	}
}

static void SendSpeedValues()
{
	double speeds[4];
	MC_getWheelSpeeds(speeds);

	//taskENTER_CRITICAL();
	if(info.pidViewer_sendTrack)
		printf("CM>TSPD;%d;%d;%d;%d;%d\r\n", TO_PRINT(speeds[2]), TO_PRINT(speeds[3]), TO_PRINT(speeds[0]), TO_PRINT(speeds[1]), TO_PRINT(PCP_GetPIDError()));
	else
		printf("CM>SPD;%d;%d;%d;%d\r\n", TO_PRINT(speeds[2]), TO_PRINT(speeds[3]), TO_PRINT(speeds[0]), TO_PRINT(speeds[1]));

	fflush(stdout);
	//taskEXIT_CRITICAL();
}
static void RunModuleServices()
{
	if(info.pidViewer.on)
	{
		if(++info.pidViewer.currTick >= info.pidViewer.tickPeriod)
		{
			info.pidViewer.currTick = 0;	// Reset counter
			SendSpeedValues();
		}
	}
	if(info.joystick.on)
	{
		// Joystick currTick resets when message is received. Therefore, tickPeriod means timeout and service shutdown
		if(++info.joystick.currTick >= info.joystick.tickPeriod)
		{
			MC_setLinearSpeed(0.0);
			MC_setAngularSpeed(0.0);
			info.joystick.currTick = 0;
			info.joystick.on = false;
			MC_SetManualMode(false);
			printf("DIAG>MSG;Joystick timeout!! \r\n");
			fflush(stdout);
		}
	}
	if(info.rtosDiag.on)
	{
		if(++info.rtosDiag.currTick >= info.rtosDiag.tickPeriod)
		{
			#if(configGENERATE_RUN_TIME_STATS)
			vTaskGetRunTimeStats( cStringBuffer );
			info.rtosDiag.currTick = 0;
			printf("RTOS>%s\r\n", cStringBuffer);
			fflush(stdout);
			#endif
		}
	}
}

static bool ProcessMessage()
{
	msg = new DiagMessage((char*)rxBuffer, rxIndex);
	if(msg->parseOk == false)
		return false;

	switch(msg->origin)
	{
		case DIAG_ORG_PIDV:
			if(info.pidViewer.on)
			{
				if(msg->id == DIAG_ID_rRPID)
				{
					double kpid[3];
					MC_getRightPIDTunings(kpid, kpid+1, kpid+2);
					printf("CM>RPID;%d;%d;%d\r\n", TO_PRINT(kpid[0]), TO_PRINT(kpid[1]), TO_PRINT(kpid[2]));
					fflush(stdout);
				}
				else if(msg->id == DIAG_ID_rLPID)
				{
					double kpid[3];
					MC_getLeftPIDTunings(kpid, kpid+1, kpid+2);
					printf("CM>LPID;%d;%d;%d\r\n", TO_PRINT(kpid[0]), TO_PRINT(kpid[1]), TO_PRINT(kpid[2]));
					fflush(stdout);
				}
				else if(msg->id == DIAG_ID_rPPID)
				{
					double kpid[3];
					PCP_getPIDTunings(kpid, kpid+1, kpid+2);
					printf("CM>PPID;%d;%d;%d\r\n", TO_PRINT(kpid[0]), TO_PRINT(kpid[1]), TO_PRINT(kpid[2]));
					fflush(stdout);
				}
				else if(msg->id == DIAG_ID_wPPID)
				{
					PCP_setPIDTunings(msg->values[0], msg->values[1], msg->values[2]);
				}
				else if(msg->id == DIAG_ID_wRPID)
				{
					MC_setRightPIDTunings(msg->values[0], msg->values[1], msg->values[2]);
				}
				else if(msg->id == DIAG_ID_wLPID)
				{
					MC_setLeftPIDTunings(msg->values[0], msg->values[1], msg->values[2]);
				}
				else if(msg->id == DIAG_ID_MOD_STOP)
				{
					if(info.joystick.on == false)	// Then timer must be stopped
					{
						configASSERT(xTimerStop(xDiagTimerHandle, 0) == pdPASS);
						timerIsOn = false;
					}
					info.pidViewer.on = false;
				}
				else if(msg->id == DIAG_ID_FILT)
				{
					MC_SetFilterState((msg->values[0] == 1.0) ? true : false);
					printf("CM>FILT;%s\r\n", (MC_GetFilterState() ? "1.0" : "0.0"));
					fflush(stdout);
				}
				else if(msg->id == DIAG_ID_TRACK)
				{
					info.pidViewer_sendTrack = (msg->values[0] == 1.0) ? true : false;
				}
			}
			else
			{
				if(msg->id == DIAG_ID_MOD_START)
				{
					if(timerIsOn == false)	// Then timer must be started
					{
						configASSERT(xTimerReset(xDiagTimerHandle, 0) == pdPASS);
						timerIsOn = true;
					}
					info.pidViewer.on = true;
				}
			}
		break;
		case DIAG_ORG_JOY:
			if(info.joystick.on)
			{
				if(msg->id == DIAG_ID_VWSPD)
				{
					MC_setLinearSpeed(msg->values[0]);
					MC_setAngularSpeed(msg->values[1]);	
					info.joystick.currTick = 0;
				}
				else if(msg->id == DIAG_ID_MOD_STOP)
				{
					if(info.pidViewer.on == false)	// Then timer must be stopped
					{
						configASSERT(xTimerStop(xDiagTimerHandle, 0) == pdPASS);
						timerIsOn = false;
					}
					info.joystick.on = false;
					MC_SetManualMode(false);
				}
			}
			else
			{
				if(msg->id == DIAG_ID_MOD_START)
				{
					if(timerIsOn == false)	// Then timer must be started
					{
						configASSERT(xTimerReset(xDiagTimerHandle, 0) == pdPASS);
						timerIsOn == true;
					}
					info.joystick.on = true;
					MC_SetManualMode(true);
				}
			}
		break;
		case DIAG_ORG_RTOSDIAG:
			if(info.rtosDiag.on)
			{
				if(msg->id == DIAG_ID_MOD_STOP)
					info.rtosDiag.on = false;

			#if(configGENERATE_RUN_TIME_STATS)
				else if(msg->id == DIAG_ID_STAT_RT)
				{
						vTaskGetRunTimeStats( cStringBuffer );
						printf("RTOS>\r%s\r\n", cStringBuffer);
						fflush(stdout);
				}
				else if(msg->id == DIAG_ID_STAT_TASK)
				{
						vTaskList( cStringBuffer );
						printf("RTOS>\r%s\r\n", cStringBuffer);
						fflush(stdout);
				}
			#endif
			
			}
			else
			{
				if(msg->id == DIAG_ID_MOD_START)
				{
					info.rtosDiag.on = true;
				}
			}

		break;
	}


	return true;
}

static void uartRxCallback(void)
{
	NVIC_ClearPendingIRQ(USART2_IRQn);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	bool eof = false;

	// Read fifo until empty
	while( rxIndex < RX_BUFFER_SIZE && uartReadByte(UART_USB, &rxBuffer[rxIndex]))
		rxIndex++;


	if(rxBuffer[rxIndex-1]== '\n' || rxIndex == RX_BUFFER_SIZE)	// End of message detected or max msg length
	{
		eof = true;
		rxBuffer[ (rxIndex < RX_BUFFER_SIZE ? rxIndex : (RX_BUFFER_SIZE-1)) ] = 0; // Always end with /0
		if(rxIndex>0 && rxIndex < RX_BUFFER_SIZE)	// If any data was read, notify main task. If buffer is full, dont notify cause msg will be lost on next interrupt.
			configASSERT( xTaskNotifyFromISR(xMainTaskToNotify, rxIndex, eSetValueWithOverwrite, &xHigherPriorityTaskWoken) == pdPASS);
		rxIndex = 0;
	}
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
