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
#define MSG_TERMINATOR '\r'
#define LOOPBACK_MODE 0 //Poner en 1 para modo loopback


/*==================[internal data declaration]==============================*/
static bool_t connected=0;
static EthMsg recBuffer,sendBuffer;
static unsigned int recBufCount=0;
static QueueHandle_t recievedMailbox,sendQueue;
static bool_t msg_recv;

void readCallback(void* a);
void CCO_recieve_task();
void CCO_send_task();

/*==================[internal functions definition]==========================*/
void CCO_recieve_task()
{
	const TickType_t read_delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes
	for( ;; )
	{
		if(connected)
		{
			uartInterrupt( CCO_UART, 0);
			if(msg_recv) //Si se terminó de recibir un mensaje, se manda a la queue
			{
				msg_recv=0;
				xQueueSendToFront(recievedMailbox,&recBuffer,0 ); //Se deberia revisar si se recibe "Disconnected" que indica que se desconectó el ESP
			}
			uartInterrupt( CCO_UART, 1);
		}
		vTaskDelay( read_delay );
	}
}
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
			while(uartTxReady(CCO_UART) && *msgP != 0) //Si tengo espacio en la fifo
			{
				uartWriteByte( CCO_UART, (uint8_t)*msgP  ); //Encolo el byte en cuestión
				msgP++;
			}
			vTaskDelay( delay ); //Se blockea por un tiempo de manera de que se vacíe la fifo sin ocupar tiempo de ejecución.
		}
		while(uartTxReady(CCO_UART));
		uartWriteByte( CCO_UART, (uint8_t) MSG_TERMINATOR ); //Envío el terminador
	}
}

void readCallback(void* a) //Se llama cada vez que hubo 8 bytes entrando o un timeout
{
	uint8_t recievedByte=1;
	if(connected==0)
	{
		connected=1;
		uartReadByte(CCO_UART, &recievedByte ); //Lee el caracter
		uartWriteString( CCO_UART, CCO_ESP_HEADER); //Le manda el header
	}

	while(uartReadByte(CCO_UART, &recBuffer.array[recBufCount])) //Lee la UART hasta vaciarla
		recBufCount++;
	if(recBufCount !=0 && recBuffer.array[recBufCount-1]== MSG_TERMINATOR) //Si el ult byte recibido es un 0, se terminó la transmisión
		msg_recv=1;
}


/*==================[external functions declaration]=========================*/


bool_t CCO_init(void)
{
	uartInit( CCO_UART, CCO_UART_BAUDRATE, LOOPBACK_MODE );
	msg_recv=0;
	uartInterrupt( CCO_UART, 1);
	uartCallbackSet( CCO_UART, UART_RECEIVE,(callBackFuncPtr_t)readCallback);
	recievedMailbox=xQueueCreate( 1,sizeof(EthMsg)); //Queues para comunicación. Para recibir solo necesita 1 espacio dado que recibe un msj y se despacha al tok
	sendQueue=xQueueCreate( 1,sizeof(EthMsg)); //Para enviar puede ser que varios procesos les pidan enviar cosas
	uartWriteByte( CCO_UART, 0  ); //Le manda byte al esp para que empiece a conectarse
	xTaskCreate( CCO_recieve_task, "CCO rec task", 100	, NULL, CCO_RECIEVE_PRIORITY, NULL ); //Crea task de misión
	xTaskCreate( CCO_send_task, "CCO send task", 100	, NULL, CCO_SEND_PRIORITY, NULL ); //Crea task de misión
}

bool_t CCO_connected(void)
{
	return connected;
}

bool_t CCO_recieveMsg(EthMsg *msgP)
{
	BaseType_t bt= xQueueReceive( recievedMailbox,msgP,0); //Si hay algo en la cola lo pone en msgP
	return bt==pdPASS; //Esto dice si se leyó algo o no.
}

bool_t CCO_sendMsg(EthMsg *msg)
{
	xQueueSendToFront(sendQueue,msg,0 );
	return 1; //Debería checkearse que no se pase de la cantidad de la cola
}
