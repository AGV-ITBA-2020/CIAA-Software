#include "my_sapi.h"        // <= Biblioteca sAPI

void GpioInit();

int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	GpioInit();

	bool_t initOk = true;
	char regIndex = 0xC1;
	uint8_t buf[1] = {0};
	int charRead = 0;
   // ---------- REPETIR POR SIEMPRE --------------------------
   for( ;; )
   {
	/*  printf("DI Readings:");
	  printf("DI0=%d ; DI1=%d ; DI2=%d ; DI3=%d \r\n", gpioRead(DI0), gpioRead(DI1), gpioRead(DI2), gpioRead(DI3));
	  printf("DI4=%d ; DI5=%d ; DI6=%d ; DI7=%d \r\n", gpioRead(DI4), gpioRead(DI5), gpioRead(DI6), gpioRead(DI7));*/

	  printf("Sending to address 0x52. Reading reg ...\r\n");
	  printf("Rx=%d ; Value=%d \r\n", VL53L0X_ReadRegisters(regIndex, 1, buf), *buf);

	  for(int i =0 ; i<50000000; i++);
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



