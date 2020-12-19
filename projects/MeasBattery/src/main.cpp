/*
 * main.cpp
 *
 *  Created on: Dec 16, 2020
 *      Author: Javier
 */
//#include "my_sapi_adc.h"
#include "my_sapi.h"
#include "WatchDogTimer.h"

#define R_SERIES_VALUE 14700.0 //Valor medido de la resistencia.
#define VIN_OPAMP_TO_VBAT ((45000.0 + R_SERIES_VALUE)/15000.0)

double ADCReadToVBat(uint16_t read );

int main()
{ /*Este codigo solo sirve para ver si se esta midiendo bien. Para medir hasta los 12V de la batería sugiero
 poner una resistencia a la entrada del analoginput a usar de aprox 15k. Para el peor caso, el input al opamp
 no llega a saturar (peor caso-> 12.8V de batería, tol 10% de la resist entonces Rs=13.5K, y VinOpamp=3.28)
*/
	bool_t debugUartEnable=1;
	MySapi_BoardInit(debugUartEnable);
	uint32_t hasTimeout =  WDT_HasTimeout();
	WDT_Start();
	if(hasTimeout)
		printf("WTD TIMEOUT!! AGV WAS RESET... \r\n");

	uint32_t feeds = 0;
	while(1)
	{
		if(feeds < 20)
		{
			feeds++;
			printf("Feeding WDT. Count=%d...\r\n", WDT_GetCount());
			WDT_Feed();
		}
		else
			printf("Nothing. Count=%d...\r\n", WDT_GetCount());
		for(int i =0 ; i<20000000; i++);
	}
	return 0;
	/*adcInit(ADC_ENABLE);
	uint16_t read;
	double vBat;
	while(1)
	{
		read = adcRead(AI0);
		vBat = ADCReadToVBat(uint16_t read );
		printf("%f\n",VBat);
	}*/
}
double ADCReadToVBat(uint16_t read )
{
	double VinOpAmp= 3.3*((double)read)/(2^10-1);
	return VIN_OPAMP_TO_VBAT * VinOpAmp;
}

