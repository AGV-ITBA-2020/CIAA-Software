/*
 * ControlCenter.h
 *
 *  Created on: Sep 7, 2020
 *      Author: Javier
 */

#ifndef PROJECTS_COM_ESP_Y_OPENMV_INC_CONTROLCENTER_H_
#define PROJECTS_COM_ESP_Y_OPENMV_INC_CONTROLCENTER_H_

#include "../inc/PathControl.h"
#include "../inc/CommunicationCentre.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{CC_IDLE,CC_ON_MISSION,CC_MANUAL, CC_ERROR, CC_PAUSE, CC_LOWPOWER} CC_State;

#define CC_MAIN_PRIORITY 2

typedef enum{BUTTON_PRESS,DELAY,NONE} intraBlockEvent;


typedef struct{
	Block_details blocks[MAX_NMBR_OF_BLOCKS_IN_MISSION]; //Bloques de la misión
	intraBlockEvent intraBlockEvent[MAX_NMBR_OF_BLOCKS_IN_MISSION]; //que evento trigerea el siguiente bloque
	bool_t active,waitForIntraBlockEvent;
	unsigned int currBlock,nmbrOfBlocks;
	//Más metadata. Para probar no es nada más necesario.
} Mission;



void CC_init();



#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_CONTROLCENTER_H_ */
