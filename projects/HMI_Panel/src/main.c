
#include "config.h"        // <= Biblioteca sAPI


/* The periods assigned to the one-shot and auto-reload timers respectively. */
#define mainAUTO_RELOAD_TIMER_PERIOD	( pdMS_TO_TICKS( 500UL ) )

void GpioInit();

int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	GpioInit();

	bool_t initOk = true;


	if(initOk == true)
	{
		printf( "Starting RTOS...\r\n" );
		vTaskStartScheduler();
	}


 //  Board_Debug_Init();
   // Crear variable del tipo tick_t para contar tiempo
   tick_t timeCount   = 0;
   printf( "Loop...\r\n" );

   // ---------- REPETIR POR SIEMPRE --------------------------
   for( ;; )
   {
	  printf("DI Readings:");
	  printf("DI0=%d ; DI1=%d ; DI2=%d ; DI3=%d \r\n", gpioRead(DI0), gpioRead(DI1), gpioRead(DI2), gpioRead(DI3));
	  printf("DI4=%d ; DI5=%d ; DI6=%d ; DI7=%d \r\n", gpioRead(DI4), gpioRead(DI5), gpioRead(DI6), gpioRead(DI7));
	  for(int i =0 ; i<10000000; i++);
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



