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

bool_t CCO_Init(void);

bool_t CCO_connected(void);

bool_t CCO_recieveMsg(char * str);

bool_t CCO_sendMsg(char * str);

#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONCENTRE_H_ */
