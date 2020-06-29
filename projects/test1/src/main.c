
#include "sapi.h"        // <= Biblioteca sAPI

int main( void )
{
   // Inicializar y configurar la plataforma
	boardInit();
	adcInit(ADC_ENABLE);

 //  Board_Debug_Init();
   // Crear variable del tipo tick_t para contar tiempo
   tick_t timeCount   = 0;
   printf( "Starting...\r\n" );
   uint32_t adcReading = 0;
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
	  printf("DI Readings:");
	  printf("DI0=%d ; DI1=%d ; DI2=%d ; DI3=%d", gpioRead(DI0), gpioRead(DI1), gpioRead(DI2), gpioRead(DI3));
	  printf("DI4=%d ; DI5=%d ; DI6=%d ; DI7=%d \r\n", gpioRead(DI4), gpioRead(DI5), gpioRead(DI6), gpioRead(DI7));
	  adcReading = adcRead(AI1);
	  printf("ADC Value: %d \r\n", adcReading);
      delay(750);
   }
   return 0;
}

