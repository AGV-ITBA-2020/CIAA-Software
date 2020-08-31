#include "my_sapi.h"
#include "uart.h"        // <= Biblioteca sAPI


int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);

	uartInit(UART_232, 115200);
   // ---------- REPETIR POR SIEMPRE --------------------------
   for( ;; )
   {

	  printf("Hello world\n");

	  for(int i =0 ; i<50000000; i++);
   }
   return 0;
}


