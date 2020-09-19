/*
 * CommunicationHandling.h
 *
 *  Created on: Sep 5, 2020
 *      Author: Javier
 */

#ifndef PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONCENTRE_H_
#define PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONCENTRE_H_

#include "my_sapi_peripheral_map.h"
#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CCO_UART_BAUDRATE 9600
#define CCO_UART UART_232
#define CCO_REC_BUF_LEN 1500
#define CCO_SEND_BUF_MSGS 5
#define CCO_ESP_HEADER "AGV 1\n"

#define CCO_RECIEVE_PRIORITY 2
#define CCO_SEND_PRIORITY 1

#define ETHERNET_MCU 1500 //Cantidad máxima de bytes enviados por ethernet
#define MAX_NMBR_OF_BLOCKS_IN_MISSION 15

typedef struct{ char array[ETHERNET_MCU]; } EthMsg;


bool_t CCO_init(void); //Inicializa el centro de comunicaciones

bool_t CCO_connected(void); //Devuelve si el ESP está conectado al wifi

bool_t CCO_recieveMsg(EthMsg *msgP); //Si hay un mensaje nuevo, devuelve uno y pone en el puntero el mensaje

bool_t CCO_sendMsg(EthMsg *msg); //Se encola un mensaje para enviar por internet.

#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONCENTRE_H_ */
