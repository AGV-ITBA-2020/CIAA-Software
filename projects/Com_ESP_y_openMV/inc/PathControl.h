/*
 * RoadProcessing.h
 *
 *  Created on: Sep 3, 2020
 *      Author: Javier
 */

#ifndef ROADPROCESSING_H_
#define ROADPROCESSING_H_

#include "my_sapi_peripheral_map.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_UART_BAUDRATE 115200
#define PC_UART UART_485

typedef enum {CHECKPOINT_SLOW_DOWN, CHECKPOINT_SPEED_UP, CHECKPOINT_FORK_LEFT, CHECKPOINT_FORK_RIGHT, CHECKPOINT_STATION,CHECKPOINT_MERGE}Block_checkpoint; //Los distintos objetivos de la mision
typedef enum {BLOCK_START,BLOCK_ABORT,BLOCK_REPLACE}Block_command; //Comandos de misiones

typedef struct  {
	Block_checkpoint blockCheckpoints[MISSION_BLOCK_MAX_STEPS];
	unsigned int blockLen;
	unsigned int currStep;
}Block_details; //La información que trae cada mensaje del openMV.

typedef struct {
	Block_command com;
	Block_details md;
}Mission_Block; //Estructura de datos esperada a recibir en MissionQueue

void PC_Init(void);

void PC_setMissionBlock(Mission_Block mb);

void PC_attachQueues(QueueHandle_t errorSignalMailbox,QueueHandle_t missionStepReachedMailbox ); //Error signal devuelve el int de error y step un bool si pasó o no el paso actual de la misión.

#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_ROADPROCESSING_H_ */
