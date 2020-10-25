/*
 * Encoder.h
 *
 *  Created on: 3 sep. 2020
 *      Author: mdevo
 */

#ifndef ENCODERV2_H_
#define ENCODERV2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Encoder.h"
#if(ENCODER_VERSION == 2)


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

void EncoderV2_Init(ENCODER_CHANNEL_T ch);
uint32_t EncoderV2_GetCount(ENCODER_CHANNEL_T ch);
uint32_t EncoderV2_GetCountFiltered(ENCODER_CHANNEL_T ch,uint32_t minCount,uint32_t maxCount);
void EncoderV2_ResetCount(ENCODER_CHANNEL_T ch);
uint32_t EncoderV2_GetCountMedian(ENCODER_CHANNEL_T ch);


#endif

#ifdef __cplusplus
}
#endif

#endif /* ENCODER_H_ */
