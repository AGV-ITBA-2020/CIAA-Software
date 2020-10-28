/*
 * AgvDiagnostics.c
 *
 *  Created on: 12 oct. 2020
 *      Author: mdevo
 */

#include "AgvDiagnostics.hpp"
#include <stdio.h>
#include "DiagMessage.h"
#include "MovementControlModule.hpp"

#include "my_sapi_uart.h"

#include "task.h"
#include "timers.h"

//#include "printf.h"

using namespace std;

#define RX_BUFFER_SIZE 128
#define TICK_TIMER_BASE pdMS_TO_TICKS(200)
#define JOYSTICK_DEFAULT_TICK_TIMEOUT 5
#define PID_DEFAULT_TICK_PERIOD 2

typedef struct {
	bool on;
	uint8_t tickPeriod;	// Period of process execution
	uint8_t currTick;		// Number of ticks since last execution
} PERIODIC_SERVICE_T;

typedef struct {
	bool diagOn;
	PERIODIC_SERVICE_T pidViewer;
	PERIODIC_SERVICE_T joystick;
} DIAG_STATE_T;


TaskHandle_t xMainTaskToNotify;
TimerHandle_t xDiagTimerHandle;
uint32_t rxIndex;
uint8_t rxBuffer[RX_BUFFER_SIZE];
DIAG_STATE_T info = { false, {false, PID_DEFAULT_TICK_PERIOD}, {false, JOYSTICK_DEFAULT_TICK_TIMEOUT} };
DiagMessage * msg;

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

	configASSERT(xTaskCreate(MainTask, "AGV_DIAG_TASK", 100, NULL, 1, &xMainTaskToNotify) == pdTRUE);

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
		if( xTaskNotifyWait(0, 0, &rxSize, pdMS_TO_TICKS(200) ) == pdPASS )	// Then an event has been received
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

	taskENTER_CRITICAL();
	printf("CM>SPD;%d;%d;%d;%d\r\n", (int)(speeds[2]*100.0), (int)(speeds[3]*100.0), (int)(speeds[0]*100.0), (int)(speeds[1]*100.0));
	fflush(stdout);
	taskEXIT_CRITICAL();
}
static void RunModuleServices()
{
	if(info.pidViewer.on)
	{
		if(++info.pidViewer.currTick == info.pidViewer.tickPeriod)
		{
			info.pidViewer.currTick = 0;	// Reset counter
			SendSpeedValues();
		}
	}
	if(info.joystick.on)
	{
		// Joystick currTick resets when message is received. Therefore, tickPeriod means timeout and service shutdown
		if(++info.joystick.currTick == info.joystick.tickPeriod)
		{
			info.joystick.currTick = 0;
			info.joystick.on = false;
			printf("DIAG>MSG;Joystick timeout!! \r\n");
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
					printf("CM>RPID;1.1;2.2;3.3\r\n");
				}
				else if(msg->id == DIAG_ID_rLPID)
				{
					printf("CM>LPID;10.1;20.2;30.3\r\n");
				}
				else if(msg->id == DIAG_ID_wRPID)
				{
					printf("CM>MSG;Setting RPID: %.1f, %.1f, %.1f\r\n", msg->values[0], msg->values[1], msg->values[2]);
				}
				else if(msg->id == DIAG_ID_wLPID)
				{
					printf("CM>MSG;Setting LPID: %.1f, %.1f, %.1f\r\n", msg->values[0], msg->values[1], msg->values[2]);
				}
				else if(msg->id == DIAG_ID_MOD_STOP)
				{
					if(info.joystick.on == false)	// Then timer must be stopped
						configASSERT(xTimerStop(xDiagTimerHandle, 0) == pdPASS);
					info.pidViewer.on = false;
				}

			}
			else
			{
				if(msg->id == DIAG_ID_MOD_START)
				{
					if(info.joystick.on == false)	// Then timer must be started
						configASSERT(xTimerReset(xDiagTimerHandle, 0) == pdPASS);
					info.pidViewer.on = true;
				}
			}
		break;
		case DIAG_ORG_JOY:
			if(info.joystick.on)
			{
				if(msg->id == DIAG_ID_VWSPD)
				{
					// printf("CM>MSG;Setting V=%.1f W=%.1f\r\n", msg->values[0], msg->values[1]);
					MC_setLinearSpeed(msg->values[0]);
					MC_setAngularSpeed(msg->values[1]);
					info.joystick.currTick = 0;
				}
				else if(msg->id == DIAG_ID_MOD_STOP)
				{
					if(info.pidViewer.on == false)	// Then timer must be stopped
						configASSERT(xTimerStop(xDiagTimerHandle, 0) == pdPASS);
					info.joystick.on = false;
				}
			}
			else
			{
				if(msg->id == DIAG_ID_MOD_START)
				{
					if(info.pidViewer.on == false)	// Then timer must be started
						configASSERT(xTimerReset(xDiagTimerHandle, 0) == pdPASS);
					info.joystick.on = true;
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
