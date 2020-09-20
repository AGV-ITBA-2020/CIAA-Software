/*
 * MovementControl.c
 *
 *  Created on: Sep 14, 2020
 *      Author: Javier
 */
#include "../inc/MovementControl.h"

static float angularVelocity,velocity;

void MC_init()
{

}

void MC_setLinVel(float vel)
{
	velocity=vel;
}

void MC_setAngVel(float angVel)
{
	angularVelocity=angVel;
}
