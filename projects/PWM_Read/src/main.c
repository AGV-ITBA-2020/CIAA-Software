
#include "config.h"        // <= Biblioteca sAPI
#include "FS-X6B_Receiver.h"

void GpioInit();

int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	GpioInit();
	FSX_Init();
	printf("Arrancando");

   // ---------- REPETIR POR SIEMPRE --------------------------
   for( ;; )
   {
	/*  printf("DI Readings:");
	  printf("DI0=%d ; DI1=%d ; DI2=%d ; DI3=%d \r\n", gpioRead(DI0), gpioRead(DI1), gpioRead(DI2), gpioRead(DI3));
	  printf("DI4=%d ; DI5=%d ; DI6=%d ; DI7=%d \r\n", gpioRead(DI4), gpioRead(DI5), gpioRead(DI6), gpioRead(DI7));*/
	  if(changed)
	  {
	   printf("Counter= %d ; COMP=%d ; Capture= %d \r \n", FSX_GetTimerValue(), compute, captureValue);
	   changed = false;
	  }


	  for(int i =0 ; i<8000000; i++);
   }
   return 0;
}

void GpioInit()
{
	gpioInit(DI0, GPIO_INPUT);
	gpioInit(DI1, GPIO_INPUT);
	gpioInit(DO4, GPIO_OUTPUT);
	gpioInit(DO5, GPIO_OUTPUT);
	gpioInit(DO6, GPIO_OUTPUT);
	gpioInit(DO7, GPIO_OUTPUT);
}



