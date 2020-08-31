#include "my_sapi.h"
#include "my_sapi_delay.h"
#include "uart.h"        // <= Biblioteca sAPI



int main( void )
{
	uint8_t n=55;
	uint8_t nRec;
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	tickInit(1);

	uartInit(UART_232, 115200,true);



	uartTxWrite( UART_232, n);
	delay(500);
	bool_t res=uartReadByte(UART_232,&nRec);// Read from RX FIFO

	uartTxWrite( UART_232, n+1);
	uartTxWrite( UART_232, n+2);
	delay(500);
	res=uartReadByte(UART_232,&nRec);// Read from RX FIFO
	res=uartReadByte(UART_232,&nRec);// Read from RX FIFO





   // ---------- REPETIR POR SIEMPRE --------------------------
   for( ;; )
   {

	  printf("Hello world\n");

	  for(int i =0 ; i<50000000; i++);
   }
   return 0;
}



