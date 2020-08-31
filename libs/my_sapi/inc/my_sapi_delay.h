/*
 * my_sapi_delay.h
 *
 *  Created on: Aug 31, 2020
 *      Author: Javier
 */

#ifndef LIBS_MY_SAPI_INC_MY_SAPI_DELAY_H_
#define LIBS_MY_SAPI_INC_MY_SAPI_DELAY_H_

#include "my_sapi_datatypes.h"

/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

bool_t tickInit( tick_t tickRateMSvalue );


void delay( tick_t duration_ms ); //Blocking delay

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

#endif /* LIBS_MY_SAPI_INC_MY_SAPI_DELAY_H_ */
