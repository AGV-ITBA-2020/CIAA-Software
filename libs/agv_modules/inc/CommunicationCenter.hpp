/*
 * CommunicationCenter.hpp
 *
 *  Created on: Oct 1, 2020
 *      Author: Javier
 */

#ifndef LIBS_AGV_MODULES_INC_COMMUNICATIONCENTER_HPP_
#define LIBS_AGV_MODULES_INC_COMMUNICATIONCENTER_HPP_

#include "EthMsgHandler.h"
#include "MissionDefs.h"

typedef enum{NEW_MISSION,ABORT_MISSION,CONTINUE_MISSION, STATUS_REQ, NOT_DEF} MSG_REC_HEADER_T;
typedef enum{MISSION_ACCEPT,MISSION_DENY, AGV_STATUS, MISSION_STEP_REACHED, ERROR} MSG_SEND_HEADER_T;

void CCO_init(void);

MSG_REC_HEADER_T CCO_getMsgType();

MISSION_T CCO_getMission();

void CCO_sendMsgWithoutData(MSG_SEND_HEADER_T msg); //Para mensajes que no contemplan un campo de datos como QUEST_ACCEPT,QUEST_DENY, QUEST_STEP_REACHED

void CCO_sendStatus(AGV_STATUS_T status);

#endif /* LIBS_AGV_MODULES_INC_COMMUNICATIONCENTER_HPP_ */
