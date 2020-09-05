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

#ifdef __cplusplus
extern "C" {
#endif

#define RP_UART_BAUDRATE 115200
#define RP_UART UART_485
#define RP_MISSION_MAX_LEN 100

typedef enum {MISSION_SLOW_DOWN, MISSION_SPEED_UP, MISSION_FORK_LEFT, MISSION_FORK_RIGHT, MISSION_STATION,MISSION_MERGE}Mission_states; //Los distintos objetivos de la mision
typedef enum {MISSION_START,MISSION_ABORT,MISSION_REPLACE}Mission_command; //Comandos de misiones

typedef struct  {
  Mission_states currMission[RP_MISSION_MAX_LEN];
  unsigned int missionLen;
  unsigned int currStep;
}Mission_details; //La información que trae cada mensaje del openMV.

typedef struct {
	Mission_command com;
	Mission_details md;
}Mission_struct; //Estructura de datos esperada a recibir en MissionQueue

void RP_Init(void);

void RP_attachQueues(QueueHandle_t missionQueue,QueueHandle_t errorSignalMailbox,QueueHandle_t missionStepReachedMailbox ); //Error signal devuelve el int de error y step un bool si pasó o no el paso actual de la misión.

#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_ROADPROCESSING_H_ */
