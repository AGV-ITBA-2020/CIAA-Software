/*
 * uart.h
 *
 *  Created on: Aug 31, 2020
 *      Author: Javier
 */

#ifndef UART_H_
#define UART_H_

#include "my_sapi_peripheral_map.h"

#ifdef __cplusplus
extern "C" {
#endif



void uartInit( uartMap_t uart, uint32_t baudRate );


#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_UART_H_ */
