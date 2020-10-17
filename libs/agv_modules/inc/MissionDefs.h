/*
 * MissionDefs.h
 *
 *  Created on: 26 sep. 2020
 *      Author: mdevo
 */

#ifndef MISSIONDEFS_H_
#define MISSIONDEFS_H_

#include "my_sapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MISSION_BLOCK_MAX_STEPS 100
#define MAX_NMBR_OF_BLOCKS_IN_MISSION 15

typedef enum {IBE_BUTTON_PRESS,IBE_HOUSTON_CONTINUE, IBE_DELAY, IBE_NONE} INTER_BLOCK_EVENT_T;
typedef enum {CHECKPOINT_SLOW_DOWN, CHECKPOINT_SPEED_UP, CHECKPOINT_FORK_LEFT, CHECKPOINT_FORK_RIGHT, CHECKPOINT_STATION,CHECKPOINT_MERGE} BLOCK_CHECKPOINT_T; //Los distintos objetivos de la mision
typedef enum {BLOCK_START, BLOCK_ABORT, BLOCK_REPLACE} BLOCK_COMMAND_T; //Comandos de misiones

typedef struct  {
	BLOCK_CHECKPOINT_T blockCheckpoints[MISSION_BLOCK_MAX_STEPS];
	unsigned int distances[MISSION_BLOCK_MAX_STEPS];
	unsigned int blockLen,currStep;
}BLOCK_DETAILS_T; //La información que trae cada mensaje del openMV.

typedef struct {
	BLOCK_COMMAND_T com;
	BLOCK_DETAILS_T md;
}MISSION_BLOCK_T; //Estructura de datos esperada a recibir en MissionQueue

typedef struct{
	BLOCK_DETAILS_T blocks[MAX_NMBR_OF_BLOCKS_IN_MISSION]; //Bloques de la misión
	INTER_BLOCK_EVENT_T interBlockEvent[MAX_NMBR_OF_BLOCKS_IN_MISSION+1]; //que evento trigerea el siguiente bloque
	bool_t active, waitForInterBlockEvent;
	unsigned int currBlock,nmbrOfBlocks;
	//Más metadata. Para probar no es nada más necesario.
} MISSION_T;
typedef struct{
	bool_t inMision, waitForInterBlockEvent, error;
	unsigned int currBlock,nmbrOfBlocks;
	//Faltan poner m�s cosas todav�a
} AGV_STATUS_T;

#ifdef __cplusplus
}
#endif

#endif /* MISSIONDEFS_H_ */
