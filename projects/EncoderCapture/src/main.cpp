
#include "../inc/config.h"        // <= Biblioteca sAPI
#include "MovementControlModule.h"


double v = 0.5;
double w = 0;

void mainTask(void * ptr);

void mainTask(void * ptr)
{
	const TickType_t xDelay5s = pdMS_TO_TICKS( 5000 );
	for( ;; )
	{
		MC_setLinearSpeed(v);
		MC_setAngularSpeed(w);
		vTaskDelay( xDelay5s );
	}
}

int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	MC_Init();

	BaseType_t debug = xTaskCreate( mainTask, "main task test", 100	, NULL, 1, NULL );

	vTaskStartScheduler();

	bool_t initOk = true;
	char regIndex = 0xC1;
	uint8_t buf[1] = {0};
	int charRead = 0;
   
   return 0;
}
