/*
 * testMergeCCO_MC.c
 *
 *  Created on: Sep 19, 2020
 *      Author: Javier
 */


#include "CommunicationCentre.h"
#include "task.h"
#include "my_sapi.h"
#include <string.h>

void mainTask();
bool_t isHeader(EthMsg * msg,char * header, char * p2Data);
EthMsg buf;

void mainTask()
{
	const TickType_t delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes
	char * p2Data;
	char speedHeader[]="Fix speed";
	for( ;; )
	{
		if(CCO_connected()) //Si estoy conectado
		{
			if(CCO_recieveMsg(&buf) && isHeader(&buf,speedHeader, p2Data)) //Si hay un mensaje nuevo, parseo las nuevas velocidades.
			{
				//Parseo las velocidades y se las paso a MC
			}
		}
		vTaskDelay( delay );
	}
}

bool_t isHeader(EthMsg * msg,char * header, char * p2Data)
{
	bool_t retVal= (msg->array == strstr (msg->array,header)); //Si el header es lo primero que llega el mensaje
	if(retVal) //Si hay header, devuelve el puntero al dato
		p2Data = msg->array + strlen(header); //El puntero a datos es donde terminan los headers
	return retVal;
}

void main(void)
{
	MySapi_BoardInit(true);
	CCO_init();
	//MC_init();
	printf("Hola Mundo!");
	xTaskCreate( mainTask, "CCO rec task", 100	, NULL, CCO_RECIEVE_PRIORITY, NULL ); //Task para debuggear lo enviado
	vTaskStartScheduler();

}



