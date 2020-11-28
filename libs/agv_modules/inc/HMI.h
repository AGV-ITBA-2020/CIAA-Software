/*
 * HMI.h
 *
 *  Created on: 20 ago. 2020
 *      Author: mdevo
 */

#ifndef HMI_H_
#define HMI_H_
/* Typedefs*/
#include "my_sapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HMI_REFRESH_MS 50

typedef enum{	
	HMI_INPUT,
	HMI_OUTPUT
}HMI_IO_TYPE; // MAX LENGTH IS DEFINED BY IO STRUCT IN HMI.C
typedef enum{
	INPUT_BUT_GREEN,
	INPUT_BUT_BLUE,
	INPUT_SW_AUTO,
	INPUT_SW_MANUAL,
	INPUT_TOTAL_COUNT
}HMI_INPUT_ID; // MAX LENGTH IS DEFINED BY IO STRUCT IN HMI.C

typedef enum{	
	OUTPUT_BUT_GREEN,
	OUTPUT_BUT_BLUE,
	OUTPUT_LEDSTRIP_RIGHT,
	OUTPUT_LEDSTRIP_LEFT,
	OUTPUT_LEDSTRIP_STOP,
	OUTPUT_BUZZER,
	OUTPUT_TOTAL_COUNT
}HMI_OUTPUT_ID; // MAX LENGTH IS DEFINED BY IO STRUCT IN HMI.C

typedef enum{
	COUNTER,
	LONG_PRESS,
	SHORT_PRESS
} HMI_INPUT_PATTERN;

typedef struct {
	HMI_IO_TYPE IO_type;		// 0 indicates an input. 1 indicates an output.
	HMI_INPUT_ID id;
	HMI_INPUT_PATTERN pattern : 3;					// Indicates the type of pattern to capture.
	unsigned int maxCount;							// Indicates the time required to consider that the button has been pressed. If 0, input is not active.
	unsigned int patCount : 3;						// Used to count patterns.
	unsigned int count;								// Used to count time pressed.
	bool_t lastValue : 1;							// Used for capturing input changes and debouncing.
	gpioMap_t inputPin;
	void (* callbackSuccess)(HMI_INPUT_ID inputId);
	void (* callbackAbort)(HMI_INPUT_ID inputId);
}HMI_Input_t;

typedef struct {
	HMI_IO_TYPE IO_type;		// 0 indicates an input. 1 indicates an output.
	gpioMap_t outputPin;
	HMI_OUTPUT_ID id;
	unsigned int timeOn;						// Indicates the # of timebases to set the output ON in one period. If 0, input is not active.
	unsigned int timeOff;					// Indicates the # of timebases to set the output OFF in one period.
	unsigned int timebaseCounter;			// Counts the timebase in each period.
	unsigned int actionCounter;			// Indicates the amount of periods to execute the pattern.
	void (* callbackSuccess)(HMI_OUTPUT_ID inputId);
	void (* callbackAbort)(HMI_OUTPUT_ID inputId);
}HMI_Output_t;

/* Funciones */
void HMI_Init();

bool_t HMI_AddToQueue(void * HMI_InputOrOutput);

gpioMap_t HMI_getCorrespondingPin(HMI_IO_TYPE IOType, unsigned int id);

void HMI_SetOutputPin(gpioMap_t pin, bool_t state);

#ifdef __cplusplus
}
#endif


#endif /* HMI_H_ */
