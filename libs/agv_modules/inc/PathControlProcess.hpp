/*
 * PathControlProcess.h
 *
 *  Created on: Oct 10, 2020
 *      Author: Sebastian
 */

#ifndef PATH_CONTROL_PROCESS_H_
#define PATH_CONTROL_PROCESS_H_

/*==================[inclusions]=============================================*/
#include "MissionDefs.h"
#include "my_sapi_peripheral_map.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "config.h"

/*==================[macros and definitions]=================================*/
#define PC_UART_BAUDRATE 115200
#define PC_UART UART_485

typedef enum {PC_BLOCK_FINISHED, PC_STEP_REACHED,PC_ERROR} PC_Event; //Comandos de misiones

class PathControlProcess_t{
    public:
        PathControlProcess_t(){};
    private:
}

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
void PC_Init(void);

void PC_setMissionBlock(MISSION_BLOCK_T mb);

bool_t PC_hasEvent();

PC_Event PC_getEvent();


#endif /* PATH_CONTROL_PROCESS_H_ */
