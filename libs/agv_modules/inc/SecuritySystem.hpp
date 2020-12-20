/*
 * SecuritySystem.hpp
 *
 *  Created on: Nov 22, 2020
 *      Author: Javier
 */

#ifndef LIBS_AGV_MODULES_INC_SECURITYSYSTEM_HPP_
#define LIBS_AGV_MODULES_INC_SECURITYSYSTEM_HPP_

#include "my_sapi_datatypes.h"

#define USE_WATCHDOG 1

void SS_init();

bool_t SS_emergencyState();
float SS_GetBatteryLevel();


#endif /* LIBS_AGV_MODULES_INC_SECURITYSYSTEM_HPP_ */
