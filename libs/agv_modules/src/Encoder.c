/*
 * Encoder.c
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */

#include "Encoder.h"

#if(ENCODER_VERSION == 1)

#include "scu_18xx_43xx.h"
#include "gima_18xx_43xx.h"
#include "timer_18xx_43xx.h"


typedef struct {
	int gpioPort;
	int gpioPin;
	int gpioFuncNumber;
	LPC_TIMER_T * timer;
	int timerNum;
	int8_t captureNum;
	int gimaInput;
	int gimaOutput;
}enc_config_t;

enc_config_t encR = { 6, 1, 5, LPC_TIMER2, 2, 0, 2, 8};
enc_config_t encL = { 2, 5, 1, LPC_TIMER0, 0, 2, 0, 2};

volatile uint32_t captureValue;	// Variable where timer captured value is saved DURING TESTING!!!
volatile bool changed;
bool compute;

void Encoder_Init(ENCODER_CHANNEL_T ch)
{
	enc_config_t * encoder;
	if(ch == ENCODER_RIGHT)
		encoder = &encR;
	else if(ch == ENCODER_LEFT)
		encoder = &encL;

	// Configure pin to capture
	Chip_SCU_PinMux( encoder->gpioPort, encoder->gpioPin, SCU_MODE_INACT | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN, encoder->gpioFuncNumber);

	// GIMA setup
	LPC_GIMA->CAPn_IN[encoder->timerNum][encoder->captureNum] = ( GIMA_ALT_SELECT(encoder->gimaInput) | GIMA_ENABLE_PULSE(0) | GIMA_ENABLE_SYNCH(1));

	// Enable timer clock and reset it
	Chip_TIMER_Init(encoder->timer);
	Chip_TIMER_PrescaleSet(encoder->timer, 1);	// Timer res = 204MHz/100 = 2.04MHz
	Chip_TIMER_SetCountClockSrc(encoder->timer, TIMER_CAPSRC_BOTH_CAPN, encoder->captureNum);
	Chip_TIMER_Enable(encoder->timer);
}

uint32_t Encoder_GetCount(ENCODER_CHANNEL_T ch)
{
	return Chip_TIMER_ReadCount( ch == ENCODER_RIGHT ? encR.timer : encL.timer);
}

void Encoder_ResetCount(ENCODER_CHANNEL_T ch)
{
	Chip_TIMER_Reset( ch == ENCODER_RIGHT ? encR.timer : encL.timer);
}

#endif
