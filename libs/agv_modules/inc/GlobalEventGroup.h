/*
 * GlobalEventGroup.h
 *
 *  Created on: Oct 8, 2020
 *      Author: Javier
 */

#ifndef LIBS_AGV_MODULES_INC_GLOBALEVENTGROUP_H_
#define LIBS_AGV_MODULES_INC_GLOBALEVENTGROUP_H_

extern EventGroupHandle_t xEventGroup;

typedef enum{ GEG_EMERGENCY_STOP, GEG_PRIORITY_STOP=1<<1, GEG_COMS_ERROR=1<<2,GEG_COMS_RX=1<<3,
	GEG_CTMOVE_FINISH=1<<4,GEG_CTMOVE_ERROR=1<<5,GEG_MISSION_PAUSED=1<<6,GEG_MISSION_ABORT=1<<7
	, GEG_LOW_POWER=1<<8,GEG_OBJECT_IN_PATH=1<<9,GEG_MISSION_STEP_REACHED=1<<9
}GEG_T;

#define SS_EVENT_MASK (GEG_EMERGENCY_STOP | GEG_PRIORITY_STOP )
#define CC_EVENT_MASK (GEG_EMERGENCY_STOP | GEG_PRIORITY_STOP | GEG_COMS_RX | GEG_COMS_ERROR)


#endif /* LIBS_AGV_MODULES_INC_GLOBALEVENTGROUP_H_ */
