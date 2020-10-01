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

#define EMH_UART_BAUDRATE 9600
#define EMH_UART UART_232
#define EMH_SEND_BUF_MSGS 5
#define EMH_REC_BUF_LEN 2
#define EMH_ESP_HEADER "AGV 1\n"
#define EMH_SEND_PRIORITY 1

#define ETHERNET_MCU 256 //Cantidad máxima de bytes enviados por ethernet (Esto debe ser tan grande como lo que se quiera mandar. Es 256 por el buf len max del ESP)

typedef struct{ char array[ETHERNET_MCU]; } EthMsg;

/*
 * @brief:	Initialize the EMH module.
 * @param:	msgRecCallback:   Callback called when a message has been recieved (when the terminator arrives)
 * 			connectionCallback:   (Not implemented yet)
 * @note:
 */
bool_t EMH_init(callBackFuncPtr_t msgRecCallback,callBackFuncPtr_t connectionCallback); //Inicializa el centro de comunicaciones
/*
 * @brief:	Returns if its connected to the internet
 * @param:
 * @note:
 */
bool_t EMH_connected(void); //Devuelve si el ESP está conectado al wifi
/*
 * @brief:	Pulls a recieved msg from the internal queue.
 * @param:	msgP:   where the msg will be pulled
 * @note:	Returns if it was put a msg in msgP
 */
bool_t EMH_recieveMsg(EthMsg *msgP); //Si hay un mensaje nuevo, devuelve uno y pone en el puntero el mensaje
/*
 * @brief:	Pushes a msg to the send queue
 * @param:	msg:   msg to send
 * @note:	Returns 0 if the queue was full (not implemented yet)
 */
bool_t EMH_sendMsg(EthMsg *msg); //Se encola un mensaje para enviar por internet.

#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONCENTRE_H_ */
