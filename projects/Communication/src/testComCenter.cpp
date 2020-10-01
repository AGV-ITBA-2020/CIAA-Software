/*
 * testComCenter.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: Javier
 */

#include "EthMsgHandler.h"


void sample(void * a);

int main(void)
{
	EMH_init((callBackFuncPtr_t) sample,(callBackFuncPtr_t) sample);
}

void sample(void * a)
{
	int * b = (int *)a;
	int c = *b;

}
