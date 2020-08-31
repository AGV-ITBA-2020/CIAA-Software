/*
 * uart.c
 *
 *  Created on: Aug 31, 2020
 *      Author: Javier
 */
#include "uart.h"
#include "chip.h"

/*==================[typedef]================================================*/

typedef struct {
   LPC_USART_T*      uartAddr;
   lpc4337ScuPin_t   txPin;
   lpc4337ScuPin_t   rxPin;
   IRQn_Type         uartIrqAddr;
} uartLpcInit_t;
/*==================[internal data declaration]==============================*/
static const uartLpcInit_t lpcUarts[] = {
// { uartAddr, { txPort, txpin, txfunc }, { rxPort, rxpin, rxfunc }, uartIrqAddr  },
   // UART_GPIO (GPIO1 = U0_TXD, GPIO2 = U0_RXD)
   { LPC_USART0, { 6, 4, FUNC2 }, { 6, 5, FUNC2 }, USART0_IRQn }, // 0
   // UART_485 (RS485/Profibus)
   { LPC_USART0, { 9, 5, FUNC7 }, { 9, 6, FUNC7 }, USART0_IRQn }, // 1
   // UART not routed
   {  LPC_UART1, { 0, 0, 0     }, { 0, 0, 0     }, UART1_IRQn  }, // 2
   // UART_USB
   { LPC_USART2, { 7, 1, FUNC6 }, { 7, 2, FUNC6 }, USART2_IRQn }, // 3
   // UART_ENET
   { LPC_USART2, { 1,15, FUNC1 }, { 1,16, FUNC1 }, USART2_IRQn }, // 4
   // UART_232
   { LPC_USART3, { 2, 3, FUNC2 }, { 2, 4, FUNC2 }, USART3_IRQn }  // 5
};
static const lpc4337ScuPin_t lpcUart485DirPin = {
   6, 2, FUNC2
};

uint8_t uartRxRead( uartMap_t uart );// Read from RX FIFO
void uartTxWrite( uartMap_t uart, const uint8_t value );

/*==================[internal functions definition]==========================*/
// Read from RX FIFO
uint8_t uartRxRead( uartMap_t uart )
{
   return Chip_UART_ReadByte( lpcUarts[uart].uartAddr );
}
// Write in TX FIFO
void uartTxWrite( uartMap_t uart, const uint8_t value )
{
   Chip_UART_SendByte( lpcUarts[uart].uartAddr, value );
}
/*==================[external functions declaration]=========================*/
void uartInit( uartMap_t uart, uint32_t baudRate, bool_t loopback )
{
   Chip_UART_Init( lpcUarts[uart].uartAddr );// Initialize UART

   Chip_UART_SetBaud( lpcUarts[uart].uartAddr, baudRate );// Set Baud rate

   // Restart FIFOS using FCR (FIFO Control Register).
   // Set Enable, Reset content, set trigger level
   Chip_UART_SetupFIFOS( lpcUarts[uart].uartAddr,
                         UART_FCR_FIFO_EN |
                         UART_FCR_TX_RS   |
                         UART_FCR_RX_RS   |
                         UART_FCR_TRG_LEV0 );
   // Dummy read
   Chip_UART_ReadByte( lpcUarts[uart].uartAddr );

   // Enable UART Transmission
   Chip_UART_TXEnable( lpcUarts[uart].uartAddr );
   if(loopback)
   {
	   Chip_UART_SetModemControl(lpcUarts[uart].uartAddr, UART_MCR_LOOPB_EN);
   }
   else
   {
	   // Configure SCU UARTn_TXD pin
	   Chip_SCU_PinMux( lpcUarts[uart].txPin.lpcScuPort,lpcUarts[uart].txPin.lpcScuPin,MD_PDN,lpcUarts[uart].txPin.lpcScuFunc );

	   // Configure SCU UARTn_RXD pin
	   Chip_SCU_PinMux( lpcUarts[uart].rxPin.lpcScuPort,lpcUarts[uart].rxPin.lpcScuPin,MD_PLN | MD_EZI | MD_ZI,lpcUarts[uart].rxPin.lpcScuFunc );
	   // Specific configurations for RS485
	     if( uart == UART_485 )
	        Chip_UART_SetRS485Flags( LPC_USART0, UART_RS485CTRL_OINV_1     ); //Esto estaba en el sapi, hay que checkear si necesita
	     	  //Se deber�a poner el pin de DE en 1 para que funcione como una uart normal
   }
}


bool_t uartReadByte( uartMap_t uart, uint8_t* receivedByte )
{
   bool_t retVal = TRUE;
   if ( uartRxReady(uart) ) {
      *receivedByte = uartRxRead(uart);
   } else {
      retVal = FALSE;
   }
   return retVal;
}

// Blocking Write 1 byte to TX FIFO
void uartWriteByte( uartMap_t uart, const uint8_t value )
{
   // Wait for space in FIFO (blocking)
   while( uartTxReady( uart ) == FALSE );
   // Send byte
   uartTxWrite( uart, value );
}


// Return TRUE if have unread data in RX FIFO
bool_t uartRxReady( uartMap_t uart )
{
   return Chip_UART_ReadLineStatus( lpcUarts[uart].uartAddr ) & UART_LSR_RDR;
}
// Return TRUE if have space in TX FIFO
bool_t uartTxReady( uartMap_t uart )
{
   return Chip_UART_ReadLineStatus( lpcUarts[uart].uartAddr ) & UART_LSR_THRE;
}


