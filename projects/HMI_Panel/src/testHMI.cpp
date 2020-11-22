
#include "config.h"        // <= Biblioteca sAPI
#include "../inc/HMI.h"

void defaultHMIInputCallback(HMI_INPUT_ID inputId);
void defaultHMIOutputCallback(HMI_OUTPUT_ID outputId);

int main( void )
{
	// Read clock settings and update SystemCoreClock variable
	MySapi_BoardInit(true);
	//Ejemplo de uso de un Output
	HMI_Output_t tempObj;
	tempObj.IO_type=HMI_OUTPUT;
	tempObj.id=OUTPUT_LEDSTRIP_STOP;
	tempObj.outputPin=HMI_getCorrespondingPin(tempObj.IO_type, tempObj.id);
	tempObj.timebaseCounter=1000;
	tempObj.timeOn=1;
	tempObj.timeOff=1;
	tempObj.actionCounter=5;
	tempObj.callbackSuccess=defaultHMIOutputCallback;
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
