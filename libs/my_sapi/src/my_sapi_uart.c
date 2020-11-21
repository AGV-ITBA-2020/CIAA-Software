/*
 * uart.c
 *
 *  Created on: Aug 31, 2020
 *      Author: Javier
 */
#include "../inc/my_sapi_uart.h"

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

/*Callbacks interrupciones*/
static callBackFuncPtr_t rxIsrCallbackUART0 = 0;
static callBackFuncPtr_t rxIsrCallbackUART2 = 0;
static callBackFuncPtr_t rxIsrCallbackUART3 = 0;

static callBackFuncPtr_t txIsrCallbackUART0 = 0;
static callBackFuncPtr_t txIsrCallbackUART2 = 0;
static callBackFuncPtr_t txIsrCallbackUART3 = 0;

/* 0x28 0x000000A0 - Handler for ISR UART0 (IRQ 24) */
void UART0_IRQHandler(void);
/* 0x2a 0x000000A8 - Handler for ISR UART2 (IRQ 26) */
void UART2_IRQHandler(void);
/* 0x2b 0x000000AC - Handler for ISR UART3 (IRQ 27) */
void UART3_IRQHandler(void);

static void uartProcessIRQ( uartMap_t uart );

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

static void uartProcessIRQ( uartMap_t uart )
{
   uint8_t status = Chip_UART_ReadLineStatus( lpcUarts[uart].uartAddr );

   // Rx Interrupt
   if(status & UART_LSR_RDR) { // uartRxReady
      // Execute callback
      if( ( uart == UART_485 ) && (rxIsrCallbackUART0 != 0) )
         (*rxIsrCallbackUART0)(0);

      if( ( uart == UART_USB )  && (rxIsrCallbackUART2 != 0) )
         (*rxIsrCallbackUART2)(0);

      if( ( uart == UART_232 )  && (rxIsrCallbackUART3 != 0) )
         (*rxIsrCallbackUART3)(0);
   }

   // Tx Interrupt
   if( ( status & UART_LSR_THRE ) && // uartTxReady
       ( Chip_UART_GetIntsEnabled( lpcUarts[uart].uartAddr ) & UART_IER_THREINT ) ) {

      // Execute callback
      if( ( uart == UART_485 ) && (txIsrCallbackUART0 != 0) )
         (*txIsrCallbackUART0)(0);

      if( ( uart == UART_USB )  && (txIsrCallbackUART2 != 0) )
         (*txIsrCallbackUART2)(0);

      if( ( uart == UART_232 )  && (txIsrCallbackUART3 != 0) )
         (*txIsrCallbackUART3)(0);
   }
}
__attribute__ ((section(".after_vectors"))) // Y esto?

// UART0 (GPIO1 y GPIO2 or RS485/Profibus)
// 0x28 0x000000A0 - Handler for ISR UART0 (IRQ 24)
void UART0_IRQHandler(void)
{
   uartProcessIRQ( UART_485 );
}

// UART2 (USB-UART) or UART_ENET
// 0x2a 0x000000A8 - Handler for ISR UART2 (IRQ 26)
void UART2_IRQHandler(void)
{
   uartProcessIRQ( UART_USB );
}

// UART3 (RS232)
// 0x2b 0x000000AC - Handler for ISR UART3 (IRQ 27)
void UART3_IRQHandler(void)
{
   uartProcessIRQ( UART_232 );
}
/*==================[external functions declaration]=========================*/
void uartInit( uartMap_t uart, uint32_t baudRate, bool_t loopback )
{
   uint32_t triggerLevel;
   Chip_UART_Init( lpcUarts[uart].uartAddr );// Initialize UART

   Chip_UART_SetBaud( lpcUarts[uart].uartAddr, baudRate );// Set Baud rate

   // Restart FIFOS using FCR (FIFO Control Register).
   // Set Enable, Reset content, set trigger level

   if(uart == UART_485 )
	   triggerLevel=UART_FCR_TRG_LEV1; //Para el openmv tiene que saltar la interrupción con 4 bytes
   else if(uart == UART_232 )
	   triggerLevel=UART_FCR_TRG_LEV2; //Cuando hay 8 bytes salta para las comunicaciones
   else
   	   triggerLevel=UART_FCR_TRG_LEV2; //Para la UART de Diagnostics
   Chip_UART_SetupFIFOS( lpcUarts[uart].uartAddr,UART_FCR_FIFO_EN | UART_FCR_TX_RS   | UART_FCR_RX_RS   |triggerLevel );
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
	   {
	        Chip_UART_SetRS485Flags( LPC_USART0, UART_RS485CTRL_OINV_1 | UART_RS485CTRL_DCTRL_EN     ); //Invertido y con dirección automática
	        Chip_SCU_PinMux( lpcUart485DirPin.lpcScuPort,lpcUart485DirPin.lpcScuPin,MD_PDN,lpcUart485DirPin.lpcScuFunc );
	   }

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

void uartWriteString( uartMap_t uart, const char* str )
{
   while( *str != 0 ) {
      uartWriteByte( uart, (uint8_t)*str );
      str++;
   }
}
//-------------------------------------------------------------
// DMA Functions
//-------------------------------------------------------------

void uartSetDMAMode(uartMap_t uart)
{
	Chip_UART_SetupFIFOS( lpcUarts[uart].uartAddr, UART_FCR_FIFO_EN | UART_FCR_TX_RS   |
	                         UART_FCR_RX_RS   |UART_FCR_TRG_LEV0 | UART_FCR_DMAMODE_SEL );

}


//-------------------------------------------------------------
// Interruptions
//-------------------------------------------------------------


// UART Global Interrupt Enable/Disable
void uartInterrupt( uartMap_t uart, bool_t enable )
{
   if( enable ) {
      // Interrupt Priority for UART channel
      NVIC_SetPriority( lpcUarts[uart].uartIrqAddr, 5 ); // FreeRTOS Requiere prioridad >= 5 (numero mas alto, mas baja prioridad)
      // Enable Interrupt for UART channel
      NVIC_EnableIRQ( lpcUarts[uart].uartIrqAddr );
   } else {
      // Disable Interrupt for UART channel
      NVIC_DisableIRQ( lpcUarts[uart].uartIrqAddr );
   }
}
void uartCallbackSet( uartMap_t uart, uartEvents_t event,callBackFuncPtr_t callbackFunc)
{
   uint32_t intMask;

   switch(event){

      case UART_RECEIVE:
         // Enable UART Receiver Buffer Register Interrupt
         //intMask = UART_IER_RBRINT;

         // Enable UART Receiver Buffer Register Interrupt and Enable UART line
         //status interrupt. LPC43xx User manual page 1118
         intMask = UART_IER_RBRINT | UART_IER_RLSINT;

         if( callbackFunc != 0 ) {
            // Set callback
            if(uart == UART_485 ){
               rxIsrCallbackUART0 = callbackFunc;
            }
            if( uart == UART_USB){
               rxIsrCallbackUART2 = callbackFunc;
            }
            if( uart == UART_232 ){
               rxIsrCallbackUART3 = callbackFunc;
            }
         } else{
            return;
         }
      break;

      case UART_TRANSMITER_FREE:
         // Enable THRE irq (TX)
         intMask = UART_IER_THREINT;

         if( callbackFunc != 0 ) {

            // Set callback
            if(uart == UART_485){
               txIsrCallbackUART0 = callbackFunc;
            }
            if(uart == UART_USB){
               txIsrCallbackUART2 = callbackFunc;
            }
            if( uart == UART_232 ){
               txIsrCallbackUART3 = callbackFunc;
            }
         } else{
            return;
         }
      break;

      default:
         return;
   }

   // Enable UART Interrupt
   Chip_UART_IntEnable(lpcUarts[uart].uartAddr, intMask);
}


