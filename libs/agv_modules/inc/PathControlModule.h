/*
 * PathControlModule.h
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */

#ifndef PATH_CONTROL_MODULE_H_
#define PATH_CONTROL_MODULE_H_


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

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
void PC_Init(void);

void PC_setMissionBlock(MISSION_BLOCK_T * mb);


#ifdef __cplusplus
}
#endif


#endif /* PATH_CONTROL_MODULE_H_ */
