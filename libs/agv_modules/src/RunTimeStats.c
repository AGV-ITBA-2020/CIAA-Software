/*
 * RunTimeStats.c
 *
 *  Created on: 23 nov. 2020
 *      Author: mdevo
 */

#include "RunTimeStats.h"
#include "timer_18xx_43xx.h"

#define RTOSDIAG_TIMER LPC_TIMER1
#define RTOSDIAG_TIMER_PRESCALER 10000			// Timer res = 204MHz/10000 = 20400Hz = 20 * RTOS TICK FREQ

void RTS_SetupTimerForRunTimeStats()
{
		// Enable timer clock and reset it
		Chip_TIMER_Init(RTOSDIAG_TIMER);
		Chip_TIMER_PrescaleSet(RTOSDIAG_TIMER, RTOSDIAG_TIMER_PRESCALER);	// Timer res = 204MHz/10000 = 20.4kHz = 20 * RTOS_TICK_FREQ
	/*	NVIC_SetPriority(RTOSDIAG_TIMER, 5 ); 	// FreeRTOS Requiere prioridad >= 5 (numero mas alto, mas baja prioridad)
		NVIC_EnableIRQ(RTOSDIAG_TIMER);	// Enable Interrupt for TIMERX channel*/
		Chip_TIMER_Enable(RTOSDIAG_TIMER);
}

uint32_t RTS_RtosGetTimerValue()
{
	return Chip_TIMER_ReadCount(RTOSDIAG_TIMER);
}