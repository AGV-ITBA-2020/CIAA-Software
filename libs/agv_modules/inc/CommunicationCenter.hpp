/*
 * CommunicationCenter.hpp
 *
 *  Created on: Oct 1, 2020
 *      Author: Javier
 */

#ifndef LIBS_AGV_MODULES_INC_COMMUNICATIONCENTER_HPP_
#define LIBS_AGV_MODULES_INC_COMMUNICATIONCENTER_HPP_


#include "MissionDefs.h"
#include "FreeRTOS.h"
#include "event_groups.h"

typedef enum{NEW_MISSION,ABORT_MISSION,CONTINUE_MISSION, STATUS_REQ, NOT_DEF} MSG_REC_HEADER_T;
typedef enum{MISSION_ACCEPT,MISSION_DENY, AGV_STATUS, MISSION_STEP_REACHED} MSG_SEND_HEADER_T;

void CCO_init(EventGroupHandle_t xEventGroup);

MSG_REC_HEADER_T CCO_getMsgType();

bool_t CCO_getMission(MISSION_T * mission);

bool_t CCO_sendMsgWithoutData(MSG_SEND_HEADER_T msg); //Para mensajes que no contemplan un campo de datos como QUEST_ACCEPT,QUEST_DENY, QUEST_STEP_REACHED

bool_t CCO_sendStatus(AGV_STATUS_T status);

#endif /* LIBS_AGV_MODULES_INC_COMMUNICATIONCENTER_HPP_ */
