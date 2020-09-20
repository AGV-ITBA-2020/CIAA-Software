/* Copyright 2020, AGV ITBA - 2020
 * 
 * Placeholder
 */

/* Date: 2020-09-12 */

/* 
 * Placeholder
 * 
 */


#ifndef _MC
#define _MC

/*==================[inclusions]=============================================*/
#include "FreeRTOS.h"
#include "queue.h"
#include "config.h"
#include "my_sapi_sct.h"
// #include "PID_v1.hpp"


#ifdef __cplusplus
extern "C"
{
#endif

/*==================[macros and definitions]=================================*/
#ifndef TICKS_TO_MS
	#define TICKS_TO_MS(xTimeInTicks) (xTimeInTicks * 100 / pdMS_TO_TICKS( 100 ))
#endif

#define AGV_AXIS_LONGITUDE 0.5
#define AGV_WHEEL_DIAMETER 0.25
#define AGV_WHEEL_RADIUS AGV_WHEEL_DIAMETER/2.0
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/*
 * @brief:	Initialize the MC module.
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void MC_Init(void);


/*
 * @brief:	Sets the linear speed for the vehicle.
 * @param:	v:   linear speed, as a double the sign defines the direction
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setLinearSpeed(double v);

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setAngularSpeed(double w);



#ifdef __cplusplus
}
#endif


#endif /* _MC */

/**
 * @}
 */
