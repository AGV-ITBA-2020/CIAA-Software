/*
 * CommunicationHandling.h
 *
 *  Created on: Sep 5, 2020
 *      Author: Javier
 */

#ifndef PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONHANDLING_H_
#define PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONHANDLING_H_

#include "my_sapi_peripheral_map.h"
#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CH_UART_BAUDRATE 9600
#define CH_UART UART_232
#define CH_REC_BUF_LEN 200

bool_t CH_Init(void);

bool_t CH_connected(void);

void CH_attach_queue(QueueHandle_t sendqueue,QueueHandle_t recmailbox);

#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_COMMUNICATIONHANDLING_H_ */
