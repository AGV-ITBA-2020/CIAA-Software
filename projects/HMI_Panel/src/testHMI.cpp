/*
#include "config.h"        // <= Biblioteca sAPI
#include "../inc/HMI.h"

void defaultHMIInputCallback(HMI_INPUT_ID inputId);
void defaultHMIOutputCallback(HMI_OUTPUT_ID outputId);

int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	HMI_Init();
	//Ejemplo de uso de un Output
	HMI_Output_t tempObj;
	tempObj.IO_type=HMI_OUTPUT;
	tempObj.id=OUTPUT_LEDSTRIP_STOP;
	tempObj.outputPin=HMI_getCorrespondingPin(tempObj.IO_type, tempObj.id);
	tempObj.timebaseCounter=0;
	tempObj.timeOn=10;
	tempObj.timeOff=10;
	tempObj.actionCounter=5;
	tempObj.callbackSuccess=defaultHMIOutputCallback;
	HMI_AddToQueue((void *)&tempObj);
	tempObj.id=OUTPUT_BUZZER;
	tempObj.outputPin=HMI_getCorrespondingPin(tempObj.IO_type, tempObj.id);
	HMI_AddToQueue((void *)&tempObj);
	HMI_Input_t exampleInput;
	exampleInput.IO_type=HMI_INPUT;
	exampleInput.id=INPUT_BUT_GREEN;
	exampleInput.pattern=LONG_PRESS;
	exampleInput.inputPin=HMI_getCorrespondingPin(exampleInput.IO_type, exampleInput.id);
	exampleInput.maxCount=1;
	exampleInput.callbackSuccess=defaultHMIInputCallback;
	HMI_AddToQueue((void *)&exampleInput);
	vTaskStartScheduler();
}

void defaultHMIInputCallback(HMI_INPUT_ID inputId)
{

	printf("Long press");
}
void defaultHMIOutputCallback(HMI_OUTPUT_ID outputId)
{

}
*/
