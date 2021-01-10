/*
 * SecuritySystem.hpp
 *
 *  Created on: Nov 22, 2020
 *      Author: Javier
 */
#include "my_sapi.h"
#include "FreeRTOS.h"
#include "GlobalEventGroup.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "wwdt_18xx_43xx.h"
#include "HMIWrapper.hpp"
#include "BMS.h"

#define USE_WATCHDOG 1

extern EventGroupHandle_t xEventGroup;
gpioMap_t emergencyPin=GPIO8;
float vBat;

void emergencyCallback();
void SS_MainTask(void *);
void SS_WdtStart();
bool_t SS_WdtHasTimeout();
uint32_t SS_WdtGetStatus();
void SS_WdtFeed();
uint32_t SS_WdtGetCount();

void SS_MainTask(void *)
{
	// Set initial variables
	TickType_t xLastTimeWoke = xTaskGetTickCount();
	TickType_t ssUpdatePeriod = pdMS_TO_TICKS(50);
	bool_t lastEmergSignalValue=0;
	for( ;; )
	{
		if(USE_WATCHDOG)
			SS_WdtFeed();	// Feed the watchdog

		vBat = BMS_GetBatteryVoltage();
		BMS_MeasureNonBlocking();

		bool_t read=gpioRead(emergencyPin);
		if(read && !lastEmergSignalValue){
			xEventGroupSetBits( xEventGroup,GEG_EMERGENCY_STOP);
			HMIW_SetOutput(OUTPUT_LEDSTRIP_STOP, true);
		}
		else if (!read && lastEmergSignalValue)
		{
			HMIW_SetOutput(OUTPUT_LEDSTRIP_STOP, false);
		}
		lastEmergSignalValue=read;
		vTaskDelayUntil(&xLastTimeWoke, ssUpdatePeriod);
	}

}

float SS_GetBatteryLevel()
{
	return vBat;
}
	

void SS_init()
{
	if(USE_WATCHDOG)
		SS_WdtStart();
	gpioInit( emergencyPin, GPIO_INPUT );
	BMS_Init();
	BMS_MeasureBlocking();	// Start a blocking measurement of the battery voltage
	xTaskCreate(SS_MainTask, "SS_TASK", 100, NULL, 1, NULL);
	//Habr�a que hacerlo con callbacks pero una paja de leer eso, no hay nada hecho
}


void emergencyCallback()
{
	xEventGroupSetBitsFromISR( xEventGroup,GEG_EMERGENCY_STOP,NULL );
}

bool_t SS_emergencyState()
{
	return gpioRead(emergencyPin);
}

void SS_WdtStart()
{
	Chip_WWDT_Init(LPC_WWDT);
	Chip_WWDT_SetTimeOut(LPC_WWDT, (uint32_t)(2*WDT_OSC));	// Watchdog set for 2 second
	Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDRESET);	// WWDT timeout will cause reset
	Chip_WWDT_Start(LPC_WWDT);
}

bool_t SS_WdtHasTimeout()
{
	if( (Chip_WWDT_GetStatus(LPC_WWDT) & WWDT_WDMOD_WDTOF) )
	{
		Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDTOF);	// Clear flag
		return true;
	}
	else
		return false;
}

uint32_t SS_WdtGetStatus()
{
	return Chip_WWDT_GetStatus(LPC_WWDT);
}

void SS_WdtFeed()
{
	Chip_WWDT_Feed(LPC_WWDT);
}

uint32_t SS_WdtGetCount()
{
	return Chip_WWDT_GetCurrentCount(LPC_WWDT);
}
