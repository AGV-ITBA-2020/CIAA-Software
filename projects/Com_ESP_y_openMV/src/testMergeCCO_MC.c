/*
 * testMergeCCO_MC.c
 *
 *  Created on: Sep 19, 2020
 *      Author: Javier
 */


#include "../inc/CommunicationCentre.h"
#include "../inc/MovementControlModule.h"
#include "task.h"
#include "my_sapi.h"
#include <stdlib.h>
#include <string.h>


static void mainTask();
static bool_t isHeader(EthMsg * msg,char * header);
EthMsg buf;

void mainTask()
{
	const TickType_t delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes
	char * p2Data;
	char * p2Space;
	double linVel,angVel;
	char speedHeader[]="Fix speed";
	for( ;; )
	{
		if(CCO_connected()) //Si estoy conectado
		{
			if(CCO_recieveMsg(&buf))
			{
				if(isHeader(&buf,speedHeader)) //Si hay un mensaje nuevo, parseo las nuevas velocidades.
				{
					p2Data = buf.array + strlen(speedHeader) +1; //El puntero a datos es donde terminan los headers. +1 Por el \n que separa header de datos
					p2Space=strchr(p2Data,' ');
					*p2Space=0x00;
					p2Space++;
					linVel=atoi(p2Data);
					angVel=atoi(p2Space);
					MC_setLinearSpeed((double)linVel*0.1*3000);
					MC_setAngularSpeed((double)angVel*0.01*3000);
				}
			}
		}
		vTaskDelay( delay );
	}
}

bool_t isHeader(EthMsg * msg,char * header)
{
	bool_t retVal= (msg->array == strstr (msg->array,header)); //Si el header es lo primero que llega el mensaje
	return retVal;
}

void main(void)
{
	MySapi_BoardInit(true);
	CCO_init();
	//MC_init();
	//Setear velocidades 0
	printf("Hola Mundo!");
	xTaskCreate( mainTask, "CCO rec task", 100	, NULL, CCO_RECIEVE_PRIORITY, NULL ); //Task para debuggear lo enviado
	vTaskStartScheduler();

}



