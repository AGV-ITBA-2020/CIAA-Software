/*
 * testCC.c
 *
 *  Created on: Sep 19, 2020
 *      Author: Javier
 */

#include "../../Communication/inc/CommunicationCentre.h"
#include "task.h"
#include "my_sapi.h"
/*
void CCO_debug();
EthMsg buf;

void CCO_debug()
{
	const TickType_t delay = pdMS_TO_TICKS( 100 ); //Delay para ver si tiene mensajes
	for( ;; )
	{
		if(CCO_connected())
		{
			if(CCO_recieveMsg(&buf)) //Si hay un mensaje nuevo, devuelve uno y pone en el puntero el mensaje
				CCO_sendMsg(&buf);
		}
		vTaskDelay( delay );
	}
}

void main(void)
{
	MySapi_BoardInit(true);
	CCO_init();
	printf("Hola Mundo!");
	xTaskCreate( CCO_debug, "CCO rec task", 100	, NULL, CCO_RECIEVE_PRIORITY, NULL ); //Task para debuggear lo enviado
	vTaskStartScheduler();

}
*/
