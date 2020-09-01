#include "my_sapi.h"
#include "my_sapi_delay.h"
#include "uart.h"        // <= Biblioteca sAPI


void examplecallback(void *);
unsigned int j=0;
uint8_t nRec;
bool_t res;

int main( void )
{
	uint8_t n=55;

	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	tickInit(1);

	uartInit(UART_232, 115200,true);



	uartTxWrite( UART_232, n);
	delay(500);
	res=uartReadByte(UART_232,&nRec);// Read from RX FIFO

	uartInterrupt( UART_232, 1 );
	uartCallbackSet( UART_232, UART_RECEIVE,(callBackFuncPtr_t) examplecallback);

	uartTxWrite( UART_232, n+1);
	uartTxWrite( UART_232, n+2);
	uartTxWrite( UART_232, n+3);
	uartTxWrite( UART_232, n+4);
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

void examplecallback(void * a)
{
	j++;
	res=uartReadByte(UART_232,&nRec);// Read from RX FIFO
	res=uartReadByte(UART_232,&nRec);// Read from RX FIFO
}

