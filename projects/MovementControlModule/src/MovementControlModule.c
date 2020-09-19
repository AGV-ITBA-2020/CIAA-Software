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

#define MAX_RPM 2700.0
#define MAX_ANGULAR_SPEED MAX_RPM*2.0*3.14/60.0
#define MAX_DUTY_CYCLE 250


/*==================[internal data declaration]==============================*/
bool testFlag;
uint8_t leftMotorOutput = 0, rightMotorOutput = 0;
double linearSpeed, angularSpeed;

//Define Variables we'll be connecting to
double leftSetpoint = 0, leftInput = 0, leftOutput = 0;
double rightSetpoint = 0, rightInput = 0, rightOutput = 0;

//Specify the links and initial tuning parameters
float Kp = 2, Ki = 0.3, Kd = 0.25;
// PID leftPID(&leftInput, &leftOutput, &leftSetpoint, Kp, Ki, Kd, DIRECT);
// PID rightPID(&rightInput, &rightOutput, &rightSetpoint, Kp, Ki, Kd, DIRECT);

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

/*
 * @brief:	cconvertRPM to dutyCycle
 * @param:	Placeholder
 * @note:	Converts RPM speed to dutyCycle value.
 */
uint8_t speedTODutyCycle(double w);

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
		
		// leftPID.compute();
		// rightPID.compute();

		leftMotorOutput = speedTODutyCycle(leftSetpoint);
		rightMotorOutput = speedTODutyCycle(rightSetpoint);

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
	leftSetpoint = -(linearSpeed + angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS;
	rightSetpoint = (linearSpeed - angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS;
}

/*
 * @brief:	cconvertRPM to dutyCycle
 * @param:	Placeholder
 * @note:	Converts RPM speed to dutyCycle value.
 */
uint8_t speedTODutyCycle(double w)
{
	return w*MAX_DUTY_CYCLE/MAX_ANGULAR_SPEED;
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

	// Turn the PID on
  	// leftPID.SetMode(AUTOMATIC);
	// rightPID.SetMode(AUTOMATIC);

  	// leftPID.SetOutputLimits(0, MAX_RPM);
	// rightPID.SetOutputLimits(0, MAX_RPM);

	MC_setLinearSpeed(1);
	MC_setAngularSpeed(1);

	xTaskCreate( mainTask, "MC Main task", 1000	, NULL, 1, NULL ); //Crea task de misión

	// TODO: Deberíamos llamar aca al vTaskStartScheduler();?
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
