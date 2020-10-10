/*
 * Encoder.c
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */
#include "EncoderV2.h"
#include "scu_18xx_43xx.h"
#include "gima_18xx_43xx.h"
#include "timer_18xx_43xx.h"

#define MAX_ENC_SAMPLES 10 //Cantidad máxima de flancos positivos que hay entre cada período de control



typedef struct {
	int gpioPort;
	int gpioPin;
	int gpioFuncNumber;
	LPC_TIMER_T * timer;
	int timerNum;
	int8_t captureNum;
	int gimaInput;
	int gimaOutput;
	unsigned int measArray[MAX_ENC_SAMPLES];
	unsigned int measCount;
	IRQn_Type  timerIrqAddr;
	uint8_t dmaChannelNum;
}enc_config_t;
/*==================[internal data declaration]==============================*/
static enc_config_t encR = { 6, 1, 5, LPC_TIMER2, 2, 0, 2, 8, {},0,0};
static enc_config_t encL = { 2, 5, 1, LPC_TIMER0, 0, 2, 0, 2, {},0,0};
enc_config_t * getEncoderPointer(ENCODER_CHANNEL_T ch);

/*==================[internal functions definition]==========================*/
enc_config_t * getEncoderPointer(ENCODER_CHANNEL_T ch)
{
	enc_config_t * retVal;
	if(ch == ENCODER_RIGHT)
		retVal = &encR;
	else if(ch == ENCODER_LEFT)
		retVal = &encL;
	return retVal;
}
/*==================[external functions declaration]=========================*/
void EncoderV2_Init(ENCODER_CHANNEL_T ch)
{
	enc_config_t * encoder=getEncoderPointer(ch);

	encoder->measCount=0;
	// Configure pin to capture
	Chip_SCU_PinMux( encoder->gpioPort, encoder->gpioPin, SCU_MODE_INACT | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INBUFF_EN, encoder->gpioFuncNumber);

	// GIMA setup
	LPC_GIMA->CAPn_IN[encoder->timerNum][encoder->captureNum] = ( GIMA_ALT_SELECT(encoder->gimaInput) | GIMA_ENABLE_PULSE(0) | GIMA_ENABLE_SYNCH(1));

	// Enable timer clock and reset it
	Chip_TIMER_Init(encoder->timer);
	Chip_TIMER_PrescaleSet(encoder->timer, 1);	// Timer res = 204MHz/100 = 2.04MHz
	Chip_TIMER_CaptureRisingEdgeEnable(encoder->timer,encoder->captureNum); //Captura los datos en flancos ascendentes

	Chip_GPDMA_Init(LPC_GPDMA);

	encoder->dmaChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_UART0_Rx );
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, 5 ); // FreeRTOS Requiere prioridad >= 5 (numero mas alto, mas baja prioridad)
	NVIC_EnableIRQ(DMA_IRQn);

	Chip_TIMER_Enable(encoder->timer);
}

uint32_t EncoderV3_GetCount(ENCODER_CHANNEL_T ch)
{
	uint32_t measAverage=0;
	enc_config_t * encoder=getEncoderPointer(ch);
	NVIC_DisableIRQ( encoder->timerIrqAddr); //Deshabilito interrupciones
	for(unsigned int i=0;i<encoder->measCount;i++) //Saca el promedio de las muestras obtenidas
		measAverage += encoder->measArray[i];
	if(encoder->measCount>0)
		measAverage /= encoder->measCount;
	NVIC_EnableIRQ( encoder->timerIrqAddr); //Habilita interrupción de vuelta
	return measAverage;
}
uint32_t EncoderV3_GetCountFiltered(ENCODER_CHANNEL_T ch,uint32_t minCount,uint32_t maxCount)
{
	uint32_t measAverage=0,totalCount=0;
	enc_config_t * encoder=getEncoderPointer(ch);
	for(unsigned int i=0;i<encoder->measCount;i++)
	{
		uint32_t meas = encoder->measArray[i];
		if(meas>=minCount && meas<=maxCount)	//Saca el promedio, pero las muestras que están fuera del rango de trabajo vuelan
			measAverage += meas;
			totalCount++;
	}
	if(totalCount>0)
		measAverage /= totalCount;
	return measAverage;
}

void EncoderV3_ResetCount(ENCODER_CHANNEL_T ch)
{
	Chip_TIMER_Reset( ch == ENCODER_RIGHT ? encR.timer : encL.timer);
}
