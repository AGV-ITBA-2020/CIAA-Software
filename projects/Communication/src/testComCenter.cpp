/*
 * testComCenter.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: Javier
 */

#include "CommunicationCenter.hpp"


EventGroupHandle_t xEventGroup;

int main(void)
{
	CCO_init(xEventGroup);
	while(1);
}
