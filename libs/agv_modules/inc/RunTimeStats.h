/*
 * RunTimeStats.h
 *
 *  Created on: 23 nov. 2020
 *      Author: mdevo
 */

#ifndef RUNTIMESTATS_H_
#define RUNTIMESTATS_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "my_sapi.h"
#include "chip.h"

#define RECORD_RUNTIME_STATS 1

void RTS_SetupTimerForRunTimeStats();
uint32_t RTS_RtosGetTimerValue();

#ifdef __cplusplus
}
#endif

#endif /* RUNTIMESTATS_H_ */
