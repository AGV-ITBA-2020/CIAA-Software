/*
 * Encoder.h
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */

#ifndef ENCODERV3_H_
#define ENCODERV3_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "my_sapi.h"
#include "chip.h"

#define ENCODER_STEPS_PER_REVOLUTION 10.0

#define TIMER_CLOCK_FREQ (2.04e6)
#define COUNT_TO_SECS(x) ((x)/TIMER_CLOCK_FREQ)
#define SEC_TO_COUNT(x) ((uint32_t)((x)*TIMER_CLOCK_FREQ))


typedef enum {
	ENCODER_LEFT = 1,
	ENCODER_RIGHT = 2
}ENCODER_CHANNEL_T;

void EncoderV3_Init(ENCODER_CHANNEL_T ch);
uint32_t EncoderV3_GetCount(ENCODER_CHANNEL_T ch);
uint32_t EncoderV3_GetCountFiltered(ENCODER_CHANNEL_T ch,uint32_t minCount,uint32_t maxCount);
void EncoderV3_ResetCount(ENCODER_CHANNEL_T ch);

#ifdef __cplusplus
}
#endif

#endif /* ENCODERv3_H_ */
