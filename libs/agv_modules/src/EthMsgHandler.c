/*
 * CommunicationHandling.c
 *
 *  Created on: Sep 5, 2020
 *      Author: Javier
 */

#include "../inc/EthMsgHandler.h"

#include "../inc/my_sapi_uart.h"

#include "event_groups.h"
#include "semphr.h"

/*==================[typedef]================================================*/
#define MSG_TERMINATOR 0
#define LOOPBACK_MODE 0 //Poner en 1 para modo loopback


/*==================[internal data declaration]==============================*/
static bool_t connected=0;
static EthMsg recBuffer,sendBuffer;
static unsigned int recBufCount=0;
static QueueHandle_t recievedQueue,sendQueue;
static callBackFuncPtr_t outCallback,conCallback;

void uartCallback(void* a);
void CCO_send_task();

/*==================[internal functions definition]==========================*/

void CCO_send_task()
{
	char * msgP;
	const TickType_t delay = pdMS_TO_TICKS( 1 );
	for( ;; )
	{

		xQueueReceive(sendQueue,&sendBuffer,portMAX_DELAY); //Espera a que llegue una misión
		msgP = sendBuffer.array;
		while(*msgP != 0) //Mientras no terminé de enviar el mensaje
		{
			while(uartTxReady(EMH_UART) && *msgP != 0) //Si tengo espacio en la fifo
			{
				uartWriteByte( EMH_UART, (uint8_t)*msgP  ); //Encolo el byte en cuestión
				msgP++;
			}
			vTaskDelay( delay ); //Se blockea por un tiempo de manera de que se vacíe la fifo sin ocupar tiempo de ejecución.
		}
		while(!uartTxReady(EMH_UART));
		uartWriteByte( EMH_UART, (uint8_t) MSG_TERMINATOR ); //Envío el terminador
	}
}

void uartCallback(void* a) //Se llama cada vez que hubo 8 bytes entrando o un timeout
{
	uint8_t recievedByte=1;
	if(connected==0)
	{
		connected=1;
		uartReadByte(EMH_UART, &recievedByte ); //Lee el caracter
		uartWriteString( EMH_UART, EMH_ESP_HEADER); //Le manda el header
		uartWriteByte( EMH_UART, 0  );
	}

	while(uartReadByte(EMH_UART, &recBuffer.array[recBufCount])) //Lee la UART hasta vaciarla
		recBufCount++;
	if(recBufCount !=0 && recBuffer.array[recBufCount-1]== MSG_TERMINATOR) //Si el ult byte recibido es un 0, se terminó la transmisión
	{
		recBufCount=0;
		xQueueSendToBackFromISR(recievedQueue,&recBuffer,0 );
		outCallback(0);
	}
}


/*==================[external functions declaration]=========================*/


void EMH_init(callBackFuncPtr_t msgRecCallback,callBackFuncPtr_t connectionCallback)
{
	uartInit( EMH_UART, EMH_UART_BAUDRATE, LOOPBACK_MODE );
	outCallback = msgRecCallback;
	conCallback = connectionCallback;
	uartInterrupt( EMH_UART, 1);
	uartCallbackSet( EMH_UART, UART_RECEIVE,(callBackFuncPtr_t)uartCallback);
	recievedQueue=xQueueCreate( EMH_REC_BUF_LEN,sizeof(EthMsg)); //Queues para comunicación. Para recibir solo necesita 1 espacio dado que recibe un msj y se despacha al tok
	sendQueue=xQueueCreate( EMH_SEND_BUF_MSGS,sizeof(EthMsg)); //Para enviar puede ser que varios procesos les pidan enviar cosas
	uartWriteString( EMH_UART, "Reset"); //Resetea el ESP
	uartWriteByte( EMH_UART, 0  );
	xTaskCreate( CCO_send_task, "CCO send task", 100	, NULL, EMH_SEND_PRIORITY, NULL ); //Crea task de misión
}

bool_t EMH_connected(void)
{
	return connected;
}

bool_t EMH_recieveMsg(EthMsg *msgP)
{
	BaseType_t bt= xQueueReceive( recievedQueue,msgP,0); //Si hay algo en la cola lo pone en msgP
	return bt==pdPASS; //Esto dice si se leyó algo o no.
}

bool_t EMH_sendMsg(EthMsg *msg)
{
	xQueueSendToBack(sendQueue,msg,0 );
	return 1; //Debería checkearse que no se pase de la cantidad de la cola
}
