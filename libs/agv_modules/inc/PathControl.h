/*
 * RoadProcessing.h
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */

#ifndef ROADPROCESSING_H_
#define ROADPROCESSING_H_

#include "MissionDefs.h"
#include "my_sapi_peripheral_map.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_UART_BAUDRATE 115200
#define PC_UART UART_485

typedef enum {PC_BLOCK_FINISHED, PC_STEP_REACHED,PC_ERROR} PC_Event; //Comandos de misiones

void PC_Init(void);

void PC_setMissionBlock(Mission_Block mb);

bool_t PC_hasEvent();

PC_Event PC_getEvent();


#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_ROADPROCESSING_H_ */
