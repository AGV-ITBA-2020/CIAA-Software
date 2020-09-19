/* Copyright 2020, AGV ITBA - 2020
 * 
 * Placeholder
 */

/* Date: 2020-09-12 */

/* 
 * Placeholder
 * 
 */

/*==================[inclusions]=============================================*/
#ifdef __DEBUG__
// #include "lpc43xx_libcfg.h"
#else
#include "../inc/MovementControlModule.h"
#endif /* __DEBUG__ */

/*==================[macros and definitions]=================================*/

#define LEFT_MOTOR_OUTPUT	CTOUT9
#define RIGHT_MOTOR_OUTPUT	CTOUT8


/*==================[internal data declaration]==============================*/
bool testFlag;
uint8_t leftMotorOutput = 0, rightMotorOutput = 0;
double linearSpeed, angularSpeed;

/*==================[internal functions declaration]=========================*/
/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void setLeftMotorDutyCtcle(uint8_t value);

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void setRightMotorDutyCtcle(uint8_t value);

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
void calculateSpeeds(void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/*******Tasks*********/
void mainTask()
{
	const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
	for( ;; )
	{
		// Calculate each duty cycle from angularSpeed & linearSpeed
		calculateSpeeds();
		// Call PID compute
		setLeftMotorDutyCtcle(leftMotorOutput);
		setRightMotorDutyCtcle(rightMotorOutput);
		vTaskDelay( xDelay250ms );
	}


}

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void setLeftMotorDutyCtcle(uint8_t value)
{
	sctSetDutyCycle(LEFT_MOTOR_OUTPUT, value);
}

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void setRightMotorDutyCtcle(uint8_t value)
{
	sctSetDutyCycle(RIGHT_MOTOR_OUTPUT, value);
}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
void calculateSpeeds(void)
{
	if (!testFlag)
	{
		if (leftMotorOutput == 255){
			testFlag = !testFlag;
		}else
		{
			leftMotorOutput++;
		}
	}else
	{
		if (leftMotorOutput == 0){
			testFlag = !testFlag;
		}else
		{
			leftMotorOutput--;
		}
		
	}
	rightMotorOutput = leftMotorOutput;
}


/*==================[external functions definition]==========================*/
/*
 * @brief:	Initialize the MC module.
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void MC_Init(void)
{
	sctInit(1000);
	sctEnablePwmFor(LEFT_MOTOR_OUTPUT);
	sctEnablePwmFor(RIGHT_MOTOR_OUTPUT);

	xTaskCreate( mainTask, "MC Main task", 1000	, NULL, 1, NULL ); //Crea task de misión
}

/*
 * @brief:	Sets the linear speed for the vehicle.
 * @param:	v:   linear speed, as a double the sign defines the direction
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setLinearSpeed(double v)
{
	linearSpeed = v;
}

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setAngularSpeed(double w)
{
	angularSpeed = w;
}

/*==================[end of file]============================================*/
