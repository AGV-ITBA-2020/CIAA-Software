#ifndef __CONFIG_H_
#define __CONFIG_H_


#ifdef __cplusplus
extern "C"
{
#endif

//#define DEBUG_ENABLE 1
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "supporting_functions.h"
#include "my_sapi.h"        // <= Biblioteca sAPI
#include "my_sapi_sct.h"

// App specific includes
//#include "MovementControlModule.h"

#define CCO_MSG_REC 1 //bit del event group que indica que llegó un mensaje

/*==================[macros and definitions]=================================*/
#ifndef TICKS_TO_MS
	#define TICKS_TO_MS(xTimeInTicks) (xTimeInTicks * 100 / pdMS_TO_TICKS( 100 ))
#endif

#ifdef __cplusplus
}
#endif


#endif /* _CONFIG_H_ */

/*
 *
 *
 */
