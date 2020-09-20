/*
 * Encoder.h
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "my_sapi.h"
#include "chip.h"

volatile uint32_t captureValue;	// Variable where timer captured value is saved DURING TESTING!!!
volatile bool changed;
bool compute;

typedef enum {
	encoderLeft = 1,
	encoderRight = 2
}ENCODER_CHANNEL;

void Encoder_Init(ENCODER_CHANNEL ch);
uint32_t Encoder_GetCount(ENCODER_CHANNEL ch);

#endif /* ENCODER_H_ */
