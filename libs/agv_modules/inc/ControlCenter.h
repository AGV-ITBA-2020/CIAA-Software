/*
 * ControlCenter.h
 *
 *  Created on: Sep 7, 2020
 *      Author: Javier
 */

#ifndef PROJECTS_COM_ESP_Y_OPENMV_INC_CONTROLCENTER_H_
#define PROJECTS_COM_ESP_Y_OPENMV_INC_CONTROLCENTER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{CC_IDLE,CC_ON_MISSION,CC_MANUAL, CC_ERROR, CC_PAUSE, CC_LOWPOWER} CC_State;

#define CC_MAIN_PRIORITY 2


void CC_init();



#ifdef __cplusplus
}
#endif


#endif /* PROJECTS_COM_ESP_Y_OPENMV_INC_CONTROLCENTER_H_ */
