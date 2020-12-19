/*
 * WatchDogTimer.h
 *
 *  Created on: 19 dic. 2020
 *      Author: mdevo
 */

#ifndef WATCHDOGTIMER_H_
#define WATCHDOGTIMER_H_

#include "my_sapi_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void WDT_Start();

// No anda!!! Ver porque el flag WDTOF no me dice que se reseteo por WDT.
bool_t WDT_HasTimeout();

void WDT_Feed();

uint32_t WDT_GetCount();

uint32_t WDT_GetStatus();

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOGTIMER_H_ */
