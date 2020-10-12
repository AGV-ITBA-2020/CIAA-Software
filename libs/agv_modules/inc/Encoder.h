/*
 * Encoder.h
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "my_sapi.h"
#include "chip.h"

#define ENCODER_STEPS_PER_REVOLUTION 10.0

typedef enum {
	ENCODER_LEFT = 1,
	ENCODER_RIGHT = 2
}ENCODER_CHANNEL_T;

void Encoder_Init(ENCODER_CHANNEL_T ch);
uint32_t Encoder_GetCount(ENCODER_CHANNEL_T ch);
void Encoder_ResetCount(ENCODER_CHANNEL_T ch);

#ifdef __cplusplus
}
#endif

#endif /* ENCODER_H_ */
