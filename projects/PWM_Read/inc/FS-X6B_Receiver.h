/*
 * FS-X6B_Receiver.h
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */

#ifndef FS_X6B_RECEIVER_H_
#define FS_X6B_RECEIVER_H_

#include "my_sapi.h"

volatile uint32_t captureValue;	// Variable where timer captured value is saved DURING TESTING!!!
volatile bool changed;
bool compute;

void FSX_Init();
uint32_t FSX_GetTimerValue();

#endif /* FS_X6B_RECEIVER_H_ */
