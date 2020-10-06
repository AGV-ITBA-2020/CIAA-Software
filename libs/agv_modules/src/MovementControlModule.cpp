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
#include "MovementControlModule.h"
#include <math.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <iostream>

// using namespace std;

#endif /* __DEBUG__ */

/*==================[macros and definitions]=================================*/
#define LEFT_MOTOR_OUTPUT	CTOUT9
#define RIGHT_MOTOR_OUTPUT	CTOUT8

#define PWM_FREQUENCY 10000

#define MAX_ANGULAR_SPEED 4.0
#define MAX_DUTY_CYCLE 100.0
#define MAX_SCT_DUTY_CYCLE 250.0
#define REDUCTION_FACTOR 60.06

#define CONTROL_SAMPLE_PERIOD_MS 50.0

#define abs(x)  ( (x<0) ? -(x) : x )

using namespace pid;

/*==================[internal data declaration]==============================*/
uint8_t leftMotorOutput = 0, rightMotorOutput = 0;
double linearSpeed, angularSpeed;

AGVMovementModule_t movementModule;

/*==================[internal functions declaration]=========================*/
/*******Tasks*********/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void mcmMainTask(void * ptr);

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
double calculateInputSpeed(uint32_t value);


/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
MotorController_t::MotorController_t(gpioMap_t in1Pin, gpioMap_t in2Pin, uint8_t enableSCTOutPin, ENCODER_CHANNEL_T ch): pidController(&input, &output, &setpoint, PID_KP, PID_KI, PID_KD, DIRECT) {
	in1 = in1Pin;
	in2 = in2Pin;
	enableSCTOut = enableSCTOutPin;
	encoderCh = ch;
}

/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void MotorController_t::init(void){
	sctEnablePwmFor(enableSCTOut);

	gpioInit(in1, GPIO_OUTPUT);
	gpioInit(in2, GPIO_OUTPUT);

	gpioWrite(in1,1);
	gpioWrite(in2,0);

	Encoder_Init(encoderCh);

	// Turn the PID on
  	pidController.SetMode(AUTOMATIC);	
  	pidController.SetOutputLimits(0, MAX_DUTY_CYCLE);
	pidController.SetSampleTime(CONTROL_SAMPLE_PERIOD_MS);
}

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void MotorController_t::setMotorDutyCtcle(uint8_t value)
{
	sctSetDutyCycle(enableSCTOut, value);
}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
void MotorController_t::setMotorDirection(bool_t direction)
{
	gpioWrite(in1, direction);
	gpioWrite(in2, !direction);
}

/*
 * @brief:	cconvertRPM to dutyCycle
 * @param:	w es la velocidad angular medida en la rueda
 * @note:	Converts RPM speed to dutyCycle value.
 */
void MotorController_t::getSpeed(void)
{
	input = calculateInputSpeed(Encoder_GetCount(encoderCh));
	Encoder_ResetCount(encoderCh);
}

/*
 * @brief:	cconvertRPM to dutyCycle
 * @param:	w es la velocidad angular medida en la rueda
 * @note:	Converts RPM speed to dutyCycle value.
 */
void MotorController_t::setSpeed(void)
{

	uint8_t sctDuty = output*MAX_SCT_DUTY_CYCLE/MAX_DUTY_CYCLE;
	if(sctDuty > MAX_SCT_DUTY_CYCLE)
		sctDuty = MAX_SCT_DUTY_CYCLE;
	if(sctDuty < 0){
		setMotorDirection(true);
	}else{
		setMotorDirection(false);
	}
	setMotorDutyCtcle(abs(sctDuty));
}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
AGVMovementModule_t::AGVMovementModule_t(void): 
leftMotor(GPIO2, GPIO3, LEFT_MOTOR_OUTPUT, ENCODER_LEFT), 
rightMotor(DO4, DO5, RIGHT_MOTOR_OUTPUT, ENCODER_RIGHT) {

}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
void AGVMovementModule_t::calculateSetpoints(void)
{
	leftMotor.setpoint = (linearSpeed + angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS;
	rightMotor.setpoint = (linearSpeed - angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS;
}


/*******Tasks*********/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
void mcmMainTask(void * ptr)
{
	const TickType_t xDelay50ms = pdMS_TO_TICKS( CONTROL_SAMPLE_PERIOD_MS );
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		movementModule.calculateSetpoints();
		movementModule.leftMotor.getSpeed();
		movementModule.rightMotor.getSpeed();
		
		movementModule.rightMotor.pidController.Compute();
		movementModule.leftMotor.pidController.Compute();
		
		printf("inputSpeedL:%d, setPointSpeedL:%d, outputL:%d, ", (int)(10.0*movementModule.leftMotor.input), (int)(10.0*movementModule.leftMotor.setpoint), (int)(movementModule.leftMotor.output));
		printf("inputSpeedR:%d, setPointSpeedR:%d, outputR:%d\n", (int)(10.0*movementModule.rightMotor.input), (int)(10.0*movementModule.rightMotor.setpoint), (int)(movementModule.rightMotor.output));

		movementModule.leftMotor.setSpeed();
		movementModule.rightMotor.setSpeed();
		vTaskDelayUntil( &xLastWakeTime, xDelay50ms );
	}
}

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
double calculateInputSpeed(uint32_t value)
{
	double speed = (double) value/(ENCODER_STEPS_PER_REVOLUTION * CONTROL_SAMPLE_PERIOD_MS * 0.001 * REDUCTION_FACTOR)*2.0*3.1415;
	
	return (speed >= 2*MAX_ANGULAR_SPEED) ? MAX_ANGULAR_SPEED : speed;
}


/*==================[external functions definition]==========================*/
/*
 * @brief:	Initialize the MC module.
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void MC_Init(void)
{
	sctInit(PWM_FREQUENCY);
	movementModule.leftMotor.init();
	movementModule.rightMotor.init();

	// Create mission task
	BaseType_t debug = xTaskCreate( mcmMainTask, "MC Main task", 50	, NULL, 1, NULL );
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
