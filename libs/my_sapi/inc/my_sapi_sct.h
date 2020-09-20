/* Copyright 2020, AGV ITBA - 2020
 *
 * Placeholder
 */

/* Date: 2020-09-12 */

#ifndef _MY_SAPI_SCT_H_
#define _MY_SAPI_SCT_H_

/*==================[inclusions]=============================================*/

#include "my_sapi_datatypes.h"
#include "my_sapi_peripheral_map.h"

/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[typedef]================================================*/

/*==================[external functions declaration]=========================*/
/*
 * @brief:   Initialize the SCT peripheral with the given frequency
 * @param:   frequency:   value in Hz
 * @note:   there can only be 1 frequency in all the SCT peripheral.
 */
void sctInit(uint32_t frequency);

/*
 * @brief	Enables pwm function for the given pin
 * @param	sctNumber:   pin where the pwm signal will be generated
 */
void sctEnablePwmFor(uint8_t sctNumber);

/*
 * @brief   Converts a value in microseconds (uS = 1x10^-6 sec) to ticks
 * @param   value:   8bit value, from 0 to 255
 * @return   Equivalent in Ticks for the LPC4337
 */
uint32_t sctUint8ToTicks(uint8_t value);

/*
 * @brief:   Sets the pwm duty cycle
 * @param:	sctNumber:   pin where the pwm signal is generated
 * @param	value:   8bit value, from 0 to 255
 * @note   For the 'ticks' parameter, see function Sct_Uint8ToTicks
 */
void sctSetDutyCycle(uint8_t sctNumber, uint8_t value);

/*
 * @brief:   Gets the pwm duty cycle
 * @param:	sctNumber:   pin where the pwm signal is generated
 * @return:   duty cycle of the channel, from 0 to 255
 */
uint8_t sctGetDutyCycle(uint8_t sctNumber);

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _MY_SAPI_SCT_H_ */