/*
 * VL53L0X.h
 *
 *  Created on: 26 jul. 2020
 *      Author: mdevo
 */

#ifndef VL53L0X_H_
#define VL53L0X_H_

#include "stdint.h"

#define VL53L0X_I2C_INST I2C0
#define VL53L0X_I2C_CLK 1000

void VL53L0X_Init();
int VL53L0X_ReadRegisters(uint8_t regIndex, int len, uint8_t * buff);

#endif /* VL53L0X_H_ */
