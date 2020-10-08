
#include "../inc/config.h"        // <= Biblioteca sAPI
#include "EncoderV3.h"

void mainTask(void * ptr);

void mainTask(void * ptr)
{
	const TickType_t xDelay5s = pdMS_TO_TICKS( 5000 );
	for( ;; )
	{
		uint32_t simpleCount = EncoderV2_GetCount(ENCODER_LEFT);
		uint32_t filteredCount =EncoderV2_GetCountFiltered(ENCODER_LEFT,SEC_TO_COUNT(2e-3),SEC_TO_COUNT(50e-3));
		printf("sC= %d, fC=%d",simpleCount ,filteredCount );
		vTaskDelay( xDelay5s );
	}
}


int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	EncoderV2_Init(ENCODER_LEFT);

	BaseType_t debug = xTaskCreate( mainTask, "main task test", 100	, NULL, 1, NULL );

	vTaskStartScheduler();

   return 0;
}


/*
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

   return 0;
}*/
