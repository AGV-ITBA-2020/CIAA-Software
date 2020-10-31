/*
 * PathControlProcess.h
 *
 *  Created on: Oct 10, 2020
 *      Author: Sebastian
 */

#ifndef PATH_CONTROL_PROCESS_H_
#define PATH_CONTROL_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==================[inclusions]=============================================*/
#include "MissionDefs.h"
#include "my_sapi_peripheral_map.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "config.h"

/*==================[macros and definitions]=================================*/
#define PC_UART_BAUDRATE 115200
#define PC_UART UART_485

// class PathControlProcess_t{
//     public:
//         PathControlProcess_t(){};
//     private:
// }

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
void PCP_Init(void);
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_startNewMissionBlock(BLOCK_DETAILS_T * mb);

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void PCP_abortMissionBlock(void);

void PCP_pauseMissionBlock(void);

void PCP_continueMissionBlock(void);

void PCP_SetLinearSpeed(double v);

#ifdef __cplusplus
}
#endif


#endif /* PATH_CONTROL_PROCESS_H_ */
