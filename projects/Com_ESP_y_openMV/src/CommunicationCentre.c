/*
 * CommunicationHandling.c
 *
 *  Created on: Sep 5, 2020
 *      Author: Javier
 */

#include "../inc/CommunicationCentre.h"
#include "../inc/my_sapi_uart.h"

#include "event_groups.h"
#include "semphr.h"


/*==================[typedef]================================================*/


/*==================[internal data declaration]==============================*/
static bool_t connected=0;
static char recBuffer[CCO_REC_BUF_LEN],sendBuffer[CCO_REC_BUF_LEN];
static unsigned int recBufCount=0;
static QueueHandle_t recievedMailbox,sendQueue;

void readCallback(void* a);
void CCO_recieve_task();
void CCO_send_task();
bool_t readUartAndCheckTerminator();

/*==================[internal functions definition]==========================*/
void CCO_recieve_task()
{
	const TickType_t read_delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes
	uint8_t recievedByte=1;
	for( ;; )
	{
		if(connected)
		{
			uartInterrupt( CCO_UART, 0);
			if (readUartAndCheckTerminator()) //Si se terminó de recibir un mensaje, se manda a la queue
				xQueueSendToFront(recievedMailbox,&recBuffer,0 );
			uartInterrupt( CCO_UART, 1);
		}
		if(!connected && uartRxReady(CCO_UART)) //Cuando el esp se conectó al wifi envía un caracter cualquiera
		{
			connected=1;
			uartReadByte(CCO_UART, &recievedByte );
		}
		vTaskDelay( read_delay );
	}
}
void CCO_send_task()
{
	for( ;; )
	{
		xQueueReceive(sendQueue,&sendBuffer,portMAX_DELAY); //Espera a que llegue una misión
		uartWriteString( CCO_UART, sendBuffer);
	}
}

void readCallback(void* a)
{
	readUartAndCheckTerminator();
}

bool_t readUartAndCheckTerminator()
{
	bool_t msg_recv=0;
	while(uartReadByte(CCO_UART, &recBuffer[recBufCount]))
		recBufCount++;
	if(recBuffer[recBufCount-1]== 0)
		msg_recv=1;
	return msg_recv;
}

/*==================[external functions declaration]=========================*/


bool_t CCO_Init(void)
{
	uartInit( CCO_UART, CCO_UART_BAUDRATE, 1 );
	uartInterrupt( CCO_UART, 1);
	uartCallbackSet( CCO_UART, UART_RECEIVE,(callBackFuncPtr_t)readCallback);
	recievedMailbox=xQueueCreate( 1,sizeof(char *));
	sendQueue=xQueueCreate( CCO_SEND_BUF_MSGS,sizeof(char *));
	xTaskCreate( CCO_recieve_task, "CH rec task", 100	, NULL, 1, NULL ); //Crea task de misión
	xTaskCreate( CCO_send_task, "CH send task", 100	, NULL, 1, NULL ); //Crea task de misión
}

bool_t CCO_connected(void)
{
	return connected;
}

bool_t CCO_recieveMsg(char * str)
{
	BaseType_t bt= xQueueReceive( recievedMailbox,&str,0);
	return bt==pdPASS;
}

bool_t CCO_sendMsg(char * str)
{
	xQueueSendToFront(recievedMailbox,&str,0 );
	return 1; //Debería checkearse que no se pase de la cantidad de la cola
}
