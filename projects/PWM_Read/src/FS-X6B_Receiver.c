/*
 * FS-X6B_Receiver.c
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */
#include "FS-X6B_Receiver.h"
#include "scu_18xx_43xx.h"
#include "timer_18xx_43xx.h"
#include "gima_18xx_43xx.h"

#define FSX_TIMER LPC_TIMER0
#define FSX_TIMER_NUM 0
#define FSX_TIMER_RGU RGU_TIMER0_RST
#define FSX_TIMER_IRQn TIMER0_IRQn
#define FSX_CAPTURE_NUM 2	// Selects one of the four capture registers (channels) to store capture value
#define GIMA_OUTPUT 2
#define GIMA_INPUT 0	// pin CTIN_2

static bool detectRising;
static volatile uint32_t tempValue = 0;

void FSX_Init()
{
	// Configure pin to capture
	Chip_SCU_PinMux(2, 5, SCU_PINIO_FAST , SCU_MODE_FUNC1);

	// GIMA setup
	LPC_GIMA->CAP0_IN[FSX_TIMER_NUM][FSX_CAPTURE_NUM] = ( (0 << 4) | (1 << 3) | (1 << 2));	// Al pedo pq es el default

	/* Enable timer clock and reset it */
	Chip_TIMER_Init(FSX_TIMER);

	// Setup input capture mode
	Chip_TIMER_PrescaleSet(LPC_TIMER0, 10);	// Timer res = 204MHz/100 = 2.04MHz
	Chip_TIMER_CaptureRisingEdgeEnable(FSX_TIMER, FSX_CAPTURE_NUM);
//	Chip_TIMER_CaptureFallingEdgeEnable(FSX_TIMER, FSX_CAPTURE_NUM);
	//Chip_TIMER_CaptureFallingEdgeEnable(FSX_TIMER, FSX_CAPTURE_NUM);
	Chip_TIMER_CaptureEnableInt(FSX_TIMER, FSX_CAPTURE_NUM);
	captureValue = 0;
	detectRising = true;
	compute = false;
	changed= true;

	__NVIC_EnableIRQ(TIMER0_IRQn);
	Chip_TIMER_Enable(FSX_TIMER);
}


void TIMER0_IRQHandler(void)
{
	if(Chip_TIMER_CapturePending(FSX_TIMER, FSX_CAPTURE_NUM))
	{
		__NVIC_ClearPendingIRQ(TIMER0_IRQn);
		Chip_TIMER_ClearCapture(FSX_TIMER, FSX_CAPTURE_NUM);

	/*	tempValue = Chip_TIMER_ReadCapture(FSX_TIMER, FSX_CAPTURE_NUM);	// Only save counter
		if(tempValue != captureValue)
		{
			captureValue = tempValue;
			changed = true;
		}
		Chip_TIMER_Reset(FSX_TIMER);*/
	/*	if(firstTime)	// Only the first time...
		{
			Chip_TIMER_CaptureFallingEdgeEnable(FSX_TIMER, FSX_CAPTURE_NUM);
			tempValue = Chip_TIMER_ReadCapture(FSX_TIMER, FSX_CAPTURE_NUM);	// Save counter
			compute = true;
			firstTime = false;
		}
		else
		{
			if(compute == true)	// Then falling edge detected
			{
				compute = false;
				captureValue = tempValue - Chip_TIMER_ReadCapture(FSX_TIMER, FSX_CAPTURE_NUM); // Compute difference between rising and falling edge
				Chip_TIMER_Reset(FSX_TIMER);
			}
			else // Then rising edge detected
			{
				compute = true;
				tempValue = Chip_TIMER_ReadCapture(FSX_TIMER, FSX_CAPTURE_NUM);	// Only save counter
			}
		}*/

		if(detectRising)	// Rising edge detected. Reset and wait for falling edge
		{
			detectRising = false;
			tempValue = Chip_TIMER_ReadCapture(FSX_TIMER, FSX_CAPTURE_NUM);
			Chip_TIMER_Disable(FSX_TIMER);
			Chip_TIMER_CaptureRisingEdgeDisable(FSX_TIMER, FSX_CAPTURE_NUM);
			Chip_TIMER_CaptureFallingEdgeEnable(FSX_TIMER, FSX_CAPTURE_NUM);
			Chip_TIMER_Reset(FSX_TIMER);
			Chip_TIMER_Enable(FSX_TIMER);
		}
		else	// Falling edge detected. Save value and wait for rising edge
		{
			detectRising = true;
			tempValue -= Chip_TIMER_ReadCapture(FSX_TIMER, FSX_CAPTURE_NUM);
			if(captureValue != tempValue)
			{
				captureValue = tempValue;
				changed = true;
			}
			Chip_TIMER_Disable(FSX_TIMER);
			Chip_TIMER_CaptureRisingEdgeEnable(FSX_TIMER, FSX_CAPTURE_NUM);
			Chip_TIMER_CaptureFallingEdgeDisable(FSX_TIMER, FSX_CAPTURE_NUM);
			Chip_TIMER_Reset(FSX_TIMER);
			Chip_TIMER_Enable(FSX_TIMER);
		}
	}
}

uint32_t FSX_GetTimerValue()
{
	return Chip_TIMER_ReadCount(FSX_TIMER);
}
