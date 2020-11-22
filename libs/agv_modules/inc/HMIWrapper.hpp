/*
 * HMIWrapper.hpp
 *
 *  Created on: Nov 22, 2020
 *      Author: Javier
 */
#include "HMI.h"

#define HMIW_LONGPRESS_MS 1000
#define HMIW_MIN_PRESS_MS 100
#define HMIW_BLINK_TIME 1000
#define HMIW_SHORTPRESS_MS 100

typedef struct{
	HMI_INPUT_ID id;
	HMI_INPUT_PATTERN pat;
}HMIW_EV_INFO;

void HMIW_Init();

HMIW_EV_INFO HMIW_GetEvInfo();

void HMIW_ListenToLongPress(HMI_INPUT_ID id);

void HMIW_ListenToShortPress(HMI_INPUT_ID id);

void HMIW_ListenToMultiplePress(HMI_INPUT_ID id, unsigned int count);

void HMIW_Blink(HMI_OUTPUT_ID id, unsigned int count);
