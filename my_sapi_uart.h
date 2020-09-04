/*
 * uart.h
 *
 *  Created on: Aug 31, 2020
 *      Author: Javier
 */

#ifndef MY_SAPI_UART_H_
#define MY_SAPI_UART_H_

#include "my_sapi_peripheral_map.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
   UART_RECEIVE,
   UART_TRANSMITER_FREE
} uartEvents_t;



void uartInit( uartMap_t uart, uint32_t baudRate, bool_t loopback );

bool_t uartRxReady( uartMap_t uart );// Return TRUE if have unread data in RX FIFO

bool_t uartTxReady( uartMap_t uart );// Return TRUE if have space in TX FIFO

bool_t uartReadByte( uartMap_t uart, uint8_t* receivedByte );

void uartTxWrite( uartMap_t uart, uint8_t value );


//-------------------------------------------------------------
// Interrupts
//-------------------------------------------------------------

/*==================[ISR external functions declaration]======================*/

// UART Global Interrupt Enable/Disable
void uartInterrupt( uartMap_t uart, bool_t enable );

// UART Interrupt event Enable and set a callback
void uartCallbackSet( uartMap_t uart, uartEvents_t event,callBackFuncPtr_t callbackFunc);


#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_MY_SAPI_UART_H_ */
