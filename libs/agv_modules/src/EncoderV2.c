/*
 * Encoder.c
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */
#include "Encoder.h"

#if(ENCODER_VERSION == 2)

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
}enc_config_t;
/*==================[internal data declaration]==============================*/
static enc_config_t encR = { 6, 1, 5, LPC_TIMER2, 2, 0, 2, 8, {},0,TIMER2_IRQn};
static enc_config_t encL = { 2, 5, 1, LPC_TIMER0, 0, 2, 0, 2, {},0,TIMER0_IRQn};
enc_config_t * getEncoderPointer(ENCODER_CHANNEL_T ch);
void TIMER0_IRQHandler(void);
void TIMER2_IRQHandler(void);
void interruptRoutine(ENCODER_CHANNEL_T ch);

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
	Chip_TIMER_PrescaleSet(encoder->timer, 100);	// Timer res = 204MHz/100 = 2.04MHz
	Chip_TIMER_CaptureRisingEdgeEnable(encoder->timer,encoder->captureNum); //Captura los datos en flancos ascendentes
	Chip_TIMER_CaptureEnableInt(encoder->timer,encoder->captureNum);//Habilito un interrupt cada vez que ocurre ese flanco ascendente



	NVIC_SetPriority(encoder->timerIrqAddr, 5 ); // FreeRTOS Requiere prioridad >= 5 (numero mas alto, mas baja prioridad)
	// Enable Interrupt for TIMERX channel
	NVIC_EnableIRQ( encoder->timerIrqAddr);

	Chip_TIMER_Enable(encoder->timer);
}

uint32_t EncoderV2_GetCount(ENCODER_CHANNEL_T ch)
{
	uint32_t measAverage=0;
	enc_config_t * encoder=getEncoderPointer(ch);
	NVIC_DisableIRQ( encoder->timerIrqAddr); //Deshabilito interrupciones
	for(unsigned int i=0;i<encoder->measCount;i++) //Saca el promedio de las muestras obtenidas
		measAverage += encoder->measArray[i];
	if(encoder->measCount>0)
		measAverage /= encoder->measCount;
	encoder->measCount=0;
	NVIC_EnableIRQ( encoder->timerIrqAddr); //Habilita interrupción de vuelta
	return measAverage;
}
uint32_t EncoderV2_GetCountFiltered(ENCODER_CHANNEL_T ch,uint32_t minCount,uint32_t maxCount)
{
	uint32_t measAverage=0,totalCount=0;
	enc_config_t * encoder=getEncoderPointer(ch);
	NVIC_DisableIRQ( encoder->timerIrqAddr); //Deshabilito interrupciones
	for(unsigned int i=0;i<encoder->measCount;i++)
	{
		uint32_t meas = encoder->measArray[i];
		if(meas>=minCount && meas<=maxCount)	//Saca el promedio, pero las muestras que están fuera del rango de trabajo vuelan
			measAverage += meas;
			totalCount++;
	}
	if(totalCount>0)
		measAverage /= totalCount;
	encoder->measCount=0;
	NVIC_EnableIRQ( encoder->timerIrqAddr); //Habilita interrupción de vuelta
	return measAverage;
}
uint32_t EncoderV2_GetCountMedian(ENCODER_CHANNEL_T ch)
{
	uint32_t measAverage=0,totalCount=0,meas;
	unsigned int arrangedArray[MAX_ENC_SAMPLES];
	enc_config_t * encoder=getEncoderPointer(ch);
	NVIC_DisableIRQ( encoder->timerIrqAddr); //Deshabilito interrupciones
	totalCount =encoder->measCount;
	for(unsigned int i=0;i<totalCount;i++)
	{
		arrangedArray[i] = encoder->measArray[i];
		for(unsigned int j=i;j<totalCount;j++)
		{
			meas = encoder->measArray[j];
			if(meas<arrangedArray[i])
			{
				encoder->measArray[j]=arrangedArray[i];
				arrangedArray[i]=meas;
			}
		}
	}
	if(totalCount>0)
		measAverage = arrangedArray[(int)(totalCount/2)];
	else
		measAverage = SEC_TO_COUNT(1);	// Asumir que el tiempo entre counts es muy alto.
	encoder->measCount=0;
	NVIC_EnableIRQ( encoder->timerIrqAddr); //Habilita interrupción de vuelta
	return measAverage;
}
void EncoderV2_ResetCount(ENCODER_CHANNEL_T ch)
{
	Chip_TIMER_Reset( ch == ENCODER_RIGHT ? encR.timer : encL.timer);
}

/*==================[Interrupt]=========================*/
void TIMER0_IRQHandler(void) //Ambas rutinas de interrupt llaman a una función en común
{

	interruptRoutine(ENCODER_LEFT);
	Chip_TIMER_Reset(encL.timer); //Esto debería ir en el caso en el que no se clearee solo el capture reg (no creo que pase)

}
void TIMER2_IRQHandler(void)
{
	interruptRoutine(ENCODER_RIGHT);
	Chip_TIMER_Reset(encR.timer); //Esto debería ir en el caso en el que no se clearee solo el capture reg (no creo que pase)

}

void interruptRoutine(ENCODER_CHANNEL_T ch)
{
	enc_config_t * encoder=getEncoderPointer(ch);
	if(encoder->measCount<MAX_ENC_SAMPLES) //En el caso que no llené el buffer con mediciones ya
		encoder->measArray[(encoder->measCount)++]=Chip_TIMER_ReadCount(encoder->timer); //Pongo la medición obtenida
	Chip_TIMER_ClearCapture(encoder->timer, encoder->captureNum);
	//Acá se podría poner disable del interrupt en caso de solo tomar una medición en vez de un promedio
}

#endif
