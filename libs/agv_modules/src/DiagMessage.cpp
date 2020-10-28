/*
 * DiagMessage.cpp
 *
 *  Created on: 18 oct. 2020
 *      Author: mdevo
 */

#include "DiagMessage.h"


DiagMessage::DiagMessage(char * buffer, uint32_t size)
{
    parseOk = true;
	// Get origin string
	 string tempStr;
    char* pch = strtok(buffer, ">");
    if(pch == NULL)	// If > is at end of data, msg is incomplete.
    {
    	ParseError();
    	return;
    }	
    tempStr = pch;	// String origin
    origin = ParseOriginCode(tempStr);
    if(origin == DIAG_ORG_NONE)
    {
    	ParseError();
    	return;
    }

	 // Get id string
    pch = strtok(NULL, ";\r\n");
    tempStr = pch;	// String id
    id = ParseIdCode(tempStr);
    if(id == DIAG_ID_NONE)
    {
    	ParseError();
    	return;
    }
    // Try to get values if any
    double tempValue;
    int vIndex = 0;
    pch = strtok(NULL, ";");

    if(*pch == '\n')		//Then no params to parse
    	return;
    while(pch != NULL)
    {
        tempStr = pch;
        tempValue = stod(tempStr);
        values[vIndex++] = tempValue;
        if(vIndex >= MSG_MAX_PARAM_COUNT)	// No more data accepted
        	pch = NULL;	// End cicle
        pch = strtok(NULL, ";");
    }
}

MSG_ORIGIN_T DiagMessage::ParseOriginCode(string str)
{
	MSG_ORIGIN_T org = DIAG_ORG_NONE;
	if(str.compare("PIDV") == 0)
		org = DIAG_ORG_PIDV;
	else if(str.compare("JOY") == 0)
		org = DIAG_ORG_JOY;

	return org;
}

MSG_ID_T DiagMessage::ParseIdCode(string str)
{
	MSG_ID_T id = DIAG_ID_NONE;
	if(str.compare("rRPID") == 0)
		id = DIAG_ID_rRPID;
	else if(str.compare("rLPID") == 0)
		id = DIAG_ID_rLPID;
	else if(str.compare("wRPID") == 0)
		id = DIAG_ID_wRPID;
	else if(str.compare("wLPID") == 0)
		id = DIAG_ID_wLPID;
	else if(str.compare("CMD") == 0)
		id = DIAG_ID_CMD;
	else if(str.compare("VWSPD") == 0)
		id = DIAG_ID_VWSPD;
    else if(str.compare("WHSPD") == 0)
        id = DIAG_ID_WHSPD;
	else if(str.compare("STOP") == 0)
		id = DIAG_ID_MOD_STOP;
	else if(str.compare("START") == 0)
		id = DIAG_ID_MOD_START;
    else if(str.compare("FILT") == 0)
        id = DIAG_ID_FILT;

	return id;
}

void DiagMessage::ParseError()
{
	parseOk=false;
}
