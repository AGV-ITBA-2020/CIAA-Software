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
#include <string>

typedef enum{CCO_NEW_MISSION,CCO_ABORT_MISSION,CCO_CONTINUE,CCO_PAUSE_MISSION, CCO_STATUS_REQ, CCO_NOT_DEF,CCO_SET_VEL} MSG_REC_HEADER_T; //All the headers that could be recieved
typedef enum{CCO_MISSION_ACCEPT=10,CCO_MISSION_DENY, CCO_AGV_STATUS, CCO_MISSION_STEP_REACHED, CCO_IBE_RECIEVED, CCO_EMERGENCY_STOP, CCO_PRIORITY_STOP} MSG_SEND_HEADER_T; //All the headers that could be sent

using namespace std;


/*
 * @brief:	Initializes the communication center
 * @param:	xEventGroup:   Event group where it publishes the given events
 * @note:	Initializes EthMsg
 */
void CCO_init();
bool_t CCO_connected();
/*
 * @brief:	Pulls msg recieved from lower layer, and parses the header
 * @note:	Returns the header of the message
 */
MSG_REC_HEADER_T CCO_getMsgType();
/*
 * @brief:	Parses the message information into the given structure
 * @param:	mission:   Mission struct pointer to be filled
 * @note:	This function must be called when a msg has been pulled and has the header of a new mission.
 */
bool_t CCO_getMission(MISSION_T * mission);
/*
 * @brief:	...
 * @param:	...
 * @note:	...
 */
double CCO_getLinSpeed();
double CCO_getAngSpeed();
/*
 * @brief:	Function to send msgs without Data (only headers)
 * @param:	header: Header to send
 * @note:	Returns 0 if the out msg FIFO is full (shouldn't happen)
 */
bool_t CCO_sendMsgWithoutData(MSG_SEND_HEADER_T header);
/*
 * @brief:	Function to send agv status
 * @param:	status:   status of the agv
 * @note:	Returns 0 if the out msg FIFO is full (shouldn't happen)
 */
bool_t CCO_sendStatus(AGV_STATUS_T status);
/*
 * @brief:	Function to send error string
 * @param:	err:   string describing the error
 * @note:	Returns 0 if the out msg FIFO is full (shouldn't happen)
 */
bool_t CCO_sendError(string err);

#endif /* LIBS_AGV_MODULES_INC_COMMUNICATIONCENTER_HPP_ */
