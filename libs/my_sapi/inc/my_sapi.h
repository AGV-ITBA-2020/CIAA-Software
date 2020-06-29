/*
 * MySapi.h
 *
 *  Created on: 13 jun. 2020
 *      Author: mdevo
 */

#ifndef __MY_SAPI_H_
#define __MY_SAPI_H_

#define DEBUG_ENABLE 1

// Must include libraries
#include "my_sapi_datatypes.h"
#include "my_sapi_peripheral_map.h"
#include "board_api.h"

// Application specific libraries
#include "my_sapi_gpio.h"                   // Use GPIO peripherals

void MySapi_BoardInit(bool_t debugUartEnable);

#endif /* __MY_SAPI_H_ */
