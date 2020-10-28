
#include "config.h"        // <= Biblioteca sAPI
#include "AgvDiagnostics.hpp"
#include "Encoder.h"


/* The periods assigned to the one-shot and auto-reload timers respectively. */
#define mainAUTO_RELOAD_TIMER_PERIOD	( pdMS_TO_TICKS( 500UL ) )

void GpioInit();

int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	GpioInit();
	Encoder_Init(ENCODER_LEFT);
	Encoder_Init(ENCODER_RIGHT);
	AgvDiag_Init();

	bool_t initOk = true;
	printf( "Starting RTOS...\r\n" );
	vTaskStartScheduler();


 //  Board_Debug_Init();
   // Crear variable del tipo tick_t para contar tiempo
   tick_t timeCount   = 0;
   printf( "Loop...\r\n" );

   // ---------- REPETIR POR SIEMPRE --------------------------
   for( ;; )
   {
	  printf("DI Readings: \r\n");
	  printf("DI0=%d ; DI1=%.3f \r\n", 15520, 13.52, gpioRead(DI2), gpioRead(DI3));
	  for(int i =0 ; i<20000000; i++);
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



