/*
 * OpenMVCom.cpp
 *
 *  Created on: Oct 21, 2020
 *      Author: Javier
 */
#include "my_sapi_uart.h"
#include "config.h"
#include "semphr.h"

static uint8_t state;
static SemaphoreHandle_t xBinarySemaphore;
void callbackInterrupt(void* a);
void masterSendTask(void * ptr);

static char recBuff[4];//Ac� se guarda la data en la interrupci�n




int main( void )
{
	MySapi_BoardInit(true);
	uartInit( UART_485, 115200, 1 );
	state=1;
	xTaskCreate( masterSendTask, "PC master send task", 100	, NULL, 1, NULL ); //Crea task de misi�n
	uartCallbackSet( UART_485, UART_RECEIVE,(callBackFuncPtr_t) callbackInterrupt);
	uartInterrupt( UART_485, 1 ); //Enables uart interrupts
	vTaskStartScheduler();
}


void PC_MissionTask()
{
	for( ;; )
	{
		xSemaphoreTake( xBinarySemaphore, portMAX_DELAY ); //Espera hasta que haya nuevos datos del openMV
		printf("b1:%d , b2:%d", recBuff[0],recBuff[1]);
	}
}





void masterSendTask(void * ptr)
{
	const TickType_t xDelay50ms = pdMS_TO_TICKS( 100 );
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		uartTxWrite(UART_485,state);
		vTaskDelayUntil( &xLastWakeTime, xDelay50ms );
	}
}


void callbackInterrupt(void* a)
{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //Seccion 6.4 sem�foros binarios freeRTOS
	for(unsigned int i=0; i<4; i++ )
		uartReadByte(UART_485,(uint8_t *) &(recBuff[i]));// Read from RX FIFO
	xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); //Esto no lo entiendo bien
}
