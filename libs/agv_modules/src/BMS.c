/*
 * BMS.c
 *
 *  Created on: 19 dic. 2020
 *      Author: mdevo
 */

#include "BMS.h"
#include "adc_18xx_43xx.h"

#define ADC_VREF (3.3)	// ADC reference voltage
#define ADC_BIT_RES 10
#define ADC_TO_VOLT(x) (((float)x)*0.0142+0.0224)
#define FILTER_ORDER 8


float valArray[FILTER_ORDER];
uint8_t filterIndex;
float battVoltage;	// Holds current mean battery voltage

void BMS_Init()
{
    ADC_CLOCK_SETUP_T ADCSetup = {
       ADC_MAX_SAMPLE_RATE,   // ADC Sample rate:ADC_MAX_SAMPLE_RATE = 400KHz
	   ADC_BIT_RES,                    // ADC resolution: ADC_10BITS = 10
       1                      // ADC Burst Mode: (true or false)
    };
	 Chip_ADC_Init( LPC_ADC0, &ADCSetup );
    /* Disable burst mode */
     Chip_ADC_SetBurstCmd( LPC_ADC0, DISABLE );
     /* Set sample rate to 200KHz */
     Chip_ADC_SetSampleRate( LPC_ADC0, &ADCSetup, ADC_MAX_SAMPLE_RATE/2 );

     Chip_ADC_EnableChannel( LPC_ADC0, ADC_CH1, ENABLE );	// Enable AIN0 CIAA input
}

void BMS_MeasureBlocking()
{
	// Start conversion
	Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
	while(Chip_ADC_ReadStatus(LPC_ADC0, ADC_CH1, ADC_DR_DONE_STAT) != SET);
}

void BMS_MeasureNonBlocking()
{
	// Start conversion
	Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
}

float BMS_GetBatteryVoltage()
{
	uint16_t adcVal = 0;
	Status stat = Chip_ADC_ReadValue( LPC_ADC0, ADC_CH1, &adcVal );
	if(stat != ERROR)
	{
		valArray[filterIndex++] = ADC_TO_VOLT(adcVal);
		if(filterIndex >= FILTER_ORDER)
			filterIndex = 0;
		battVoltage = 0;
		for(int i = 0 ; i<FILTER_ORDER ; i++)
			battVoltage += valArray[i];
		return battVoltage/FILTER_ORDER;
	}
	else
		return (-1.0);
}

uint16_t BMS_GetAdcRaw()
{
	uint16_t adcVal = 0;
	Status stat = Chip_ADC_ReadValue( LPC_ADC0, ADC_CH1, &adcVal );
	if(stat != ERROR)
		return adcVal;
	else
		return 0;
}
