/*
 * ProcDeCamino.h
 *
 *  Created on: 16 jun. 2020
 *      Author: mdevo
 */

#ifndef PROCDECAMINO_H_
#define PROCDECAMINO_H_

#include "my_sapi_datatypes.h"

enum PC_INPUT{
	PC_ADC,
	PC_RS232,
	PC_PWM
};

#define PC_TASK_PRIO 6
#define PC_INPUT_TYPE PC_ADC
#define TRACK_SAMPLE_PERIOD 500	// MS. Period between track info sampling.


#endif /* PROCDECAMINO_H_ */
