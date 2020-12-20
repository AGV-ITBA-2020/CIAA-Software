/*
 * BMS.h
 *
 *  Created on: 19 dic. 2020
 *      Author: mdevo
 */

#ifndef BMS_H_
#define BMS_H_

#include "my_sapi_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void BMS_Init();
void BMS_MeasureBlocking();
float BMS_GetBatteryVoltage();
uint16_t BMS_GetAdcRaw();
void BMS_MeasureNonBlocking();

#ifdef __cplusplus
}
#endif

#endif /* BMS_H_ */
