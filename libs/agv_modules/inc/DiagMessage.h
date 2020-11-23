/*
 * DiagMessage.h
 *
 *  Created on: 18 oct. 2020
 *      Author: mdevo
 */

#ifndef DIAGMESSAGE_H_
#define DIAGMESSAGE_H_

#include <string.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_MAX_PARAM_COUNT 4

typedef enum
{
	DIAG_ORG_NONE,
	DIAG_ORG_PIDV,
	DIAG_ORG_JOY,
	DIAG_ORG_RTOSDIAG
}MSG_ORIGIN_T;
typedef enum
{
	DIAG_ID_NONE,
	DIAG_ID_rRPID,
	DIAG_ID_rLPID,
	DIAG_ID_wRPID,
	DIAG_ID_wLPID,
	DIAG_ID_rPPID,
	DIAG_ID_wPPID,
	DIAG_ID_CMD,
	DIAG_ID_VWSPD,	// Vehicle Speed
	DIAG_ID_WHSPD,	// Wheel Speed
	DIAG_ID_MOD_STOP,
	DIAG_ID_MOD_START,
	DIAG_ID_FILT,		// Sets filter state
	DIAG_ID_TRACK,
	DIAG_ID_STAT_RT,	// RTOS runtime stats
	DIAG_ID_STAT_TASK	// RTOS task list stats
} MSG_ID_T;

using namespace std;
class DiagMessage 
{
	private:
		void ParseError();
		MSG_ORIGIN_T ParseOriginCode(string str);
		MSG_ID_T ParseIdCode(string str);
	
	public:
		bool parseOk;
		MSG_ORIGIN_T origin;
		MSG_ID_T id;
		float values[MSG_MAX_PARAM_COUNT];
		DiagMessage(char * buffer, uint32_t size);	// Initializes object, parses data and loads values. Check parseOk value to verify init.
};


#ifdef __cplusplus
}
#endif

#endif /* DIAGMESSAGE_H_ */
