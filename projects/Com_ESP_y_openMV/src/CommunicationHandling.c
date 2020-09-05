/*
 * CommunicationHandling.c
 *
 *  Created on: Sep 5, 2020
 *      Author: Javier
 */

#include "CommunicationHandling.h"
#include "../inc/my_sapi_uart.h"

#include "event_groups.h"
#include "semphr.h"


/*==================[typedef]================================================*/


/*==================[internal data declaration]==============================*/
static bool_t connected=0,msg_recv=0;
static char recBuffer[CH_REC_BUF_LEN],sendBuffer[CH_REC_BUF_LEN];
static unsigned int recBufCount=0;
static QueueHandle_t recievedMailbox,sendQueue;

void readCallback(void* a);
void CH_recieve_task();
void CH_send_task();

/*==================[internal functions definition]==========================*/
void CH_recieve_task()
{
	const TickType_t read_delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes
	uint8_t recievedByte=1;
	for( ;; )
	{
		if(connected)
		{
			if (msg_recv) //Si se terminó de recibir un mensaje, se manda a la queue
				xQueueSendToFront(recievedMailbox,&recBuffer,0 );
		}
		if(!connected && uartRxReady(CH_UART)) //Cuando el esp se conectó al wifi envía un caracter cualquiera
		{
			connected=1;
			uartReadByte(CH_UART, &recievedByte );
		}
		vTaskDelay( read_delay );
	}
}
void CH_send_task()
{
	for( ;; )
	{
		xQueueReceive(sendQueue,&sendBuffer,portMAX_DELAY); //Espera a que llegue una misión
		uartWriteString( CH_UART, sendBuffer);
	}
}

void readCallback(void* a)
{
	while(uartReadByte(CH_UART, &recBuffer[recBufCount] ))
		recBufCount++;
	if(recBuffer[recBufCount-1]== 0)
		msg_recv=1;
}

/*==================[external functions declaration]=========================*/


bool_t CH_Init(void)
{
	uartInit( CH_UART, CH_UART_BAUDRATE, 1 );
	uartInterrupt( CH_UART, 1);
	uartCallbackSet( CH_UART, UART_RECEIVE,(callBackFuncPtr_t)readCallback);
	xTaskCreate( CH_recieve_task, "CH rec task", 100	, NULL, 1, NULL ); //Crea task de misión
	xTaskCreate( CH_send_task, "CH send task", 100	, NULL, 1, NULL ); //Crea task de misión
}

bool_t CH_connected(void)
{
	return connected;
}

void CH_attach_queue(QueueHandle_t sendqueue,QueueHandle_t recmailbox)
{
	recievedMailbox=recmailbox;
	sendQueue=sendqueue;
}
