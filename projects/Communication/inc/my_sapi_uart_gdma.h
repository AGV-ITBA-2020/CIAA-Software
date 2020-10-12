/*
 * my_sapi_uart_gdma.h
 *
 *  Created on: Oct 7, 2020
 *      Author: Javier
 */

#ifndef PROJECTS_COMMUNICATION_INC_MY_SAPI_UART_GDMA_H_
#define PROJECTS_COMMUNICATION_INC_MY_SAPI_UART_GDMA_H_

#include "my_sapi_peripheral_map.h"

#ifdef __cplusplus
extern "C" {
#endif

void uartDMAInit( uartMap_t uart, uint32_t baudRate, bool_t loopback ); //Inicializa la uart deseada, se puede poner en modo loopback


#ifdef __cplusplus
}
#endif




#endif /* PROJECTS_COMMUNICATION_INC_MY_SAPI_UART_GDMA_H_ */
