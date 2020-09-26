/*
 * MissionDefs.h
 *
 *  Created on: 26 sep. 2020
 *      Author: mdevo
 */

#ifndef MISSIONDEFS_H_
#define MISSIONDEFS_H_

#include "my_sapi.h"

#define MISSION_BLOCK_MAX_STEPS 100
#define MAX_NMBR_OF_BLOCKS_IN_MISSION 15

typedef enum {BUTTON_PRESS, DELAY, NONE} Inter_Block_Event;
typedef enum {CHECKPOINT_SLOW_DOWN, CHECKPOINT_SPEED_UP, CHECKPOINT_FORK_LEFT, CHECKPOINT_FORK_RIGHT, CHECKPOINT_STATION,CHECKPOINT_MERGE}Block_Checkpoint; //Los distintos objetivos de la mision
typedef enum {BLOCK_START, BLOCK_ABORT, BLOCK_REPLACE}Block_Command; //Comandos de misiones

typedef struct  {
	Block_Checkpoint blockCheckpoints[MISSION_BLOCK_MAX_STEPS];
	unsigned int blockLen;
	unsigned int currStep;
}Block_Details; //La información que trae cada mensaje del openMV.

typedef struct {
	Block_Command com;
	Block_Details md;
}Mission_Block; //Estructura de datos esperada a recibir en MissionQueue

typedef struct{
	Block_Details blocks[MAX_NMBR_OF_BLOCKS_IN_MISSION]; //Bloques de la misión
	Inter_Block_Event interBlockEvent[MAX_NMBR_OF_BLOCKS_IN_MISSION]; //que evento trigerea el siguiente bloque
	bool_t active, waitForInterBlockEvent;
	unsigned int currBlock,nmbrOfBlocks;
	//Más metadata. Para probar no es nada más necesario.
} Mission;

#endif /* MISSIONDEFS_H_ */
