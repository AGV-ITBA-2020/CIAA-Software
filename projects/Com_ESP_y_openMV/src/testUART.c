#include "../inc/my_sapi_uart.h"        // <= Biblioteca sAPI
#include "my_sapi.h"
//#include "my_sapi_delay.h"


void examplecallback(void *);
unsigned int j=0;
uint8_t nRec;
bool_t res;
/*
int main( void )
{
	uint8_t n=55;

	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	tickInit(1);
	printf("Inicializando\n");
	uartInit(UART_485, 115200,true);



	uartTxWrite( UART_485, n);
	for(int i =0 ; i<5000; i++)
		n= n+i -2*i +i;
	res=uartReadByte(UART_485,&nRec);// Read from RX FIFO
	if(res==1 && nRec==n)
		printf("Llego bien\n");

	uartInterrupt( UART_485, 1 );
	uartCallbackSet( UART_485, UART_RECEIVE,(callBackFuncPtr_t) examplecallback);

	uartTxWrite( UART_485, n+1);
	uartTxWrite( UART_485, n+2);
	uartTxWrite( UART_485, n+3);
	uartTxWrite( UART_485, n+4);
	for(int i =0 ; i<500000000; i++)
			n= n+i -2*i +i;
	res=uartReadByte(UART_485,&nRec);// Read from RX FIFO
	res=uartReadByte(UART_485,&nRec);// Read from RX FIFO





   // ---------- REPETIR POR SIEMPRE --------------------------
   for( ;; )
   {

	  printf("Hello world\n");

	  for(int i =0 ; i<50000000; i++);
   }
   return 0;
}*/

void examplecallback(void * a) //OJO! Solo la interrupcion se baja sola al leer los datos en la fifo
{
	j++;
	res=uartReadByte(UART_485,&nRec);// Read from RX FIFO
	res=uartReadByte(UART_485,&nRec);// Read from RX FIFO
}

