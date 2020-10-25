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
#include "MovementControlModule.hpp"
// #include <iostream>
// #include <math.h>

// #include <stdlib.h>
// #include <stdio.h>


using namespace pid;

#endif /* __DEBUG__ */

/*==================[macros and definitions]=================================*/
#define LEFT_MOTOR_OUTPUT	CTOUT9
#define RIGHT_MOTOR_OUTPUT	CTOUT8

#define PWM_FREQUENCY 20000

#define MAX_ANGULAR_SPEED	5.4	// rad/s Midiendo la salida del eje de las ruedas con la fuente de 12 V sobre el motor DERECHO
#define MAX_DUTY_CYCLE		100.0
#define MAX_SCT_DUTY_CYCLE	250.0
#define REDUCTION_FACTOR	60.06

#define CONTROL_SAMPLE_PERIOD_MS 50.0

#define abs(x)  ( (x<0) ? -(x) : x )


double coeffs[ FILTER_ORDER ] =
{
//	0.0035, -0.0214, 0.0630, -0.1249, 0.1834, 0.7923, 0.1834, -0.1249, 0.0630, -0.0214
		0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1
};

/*==================[internal data declaration]==============================*/
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

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
double filter(double inputData[]);


/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
/*
 * @brief:	Main task for the movement control module
 * @param:	Placeholder
 * @note:	It basically sets the value of the pwm for both motors.
 */
MotorController_t::MotorController_t(gpioMap_t in1Pin, gpioMap_t in2Pin, uint8_t enableSCTOutPin, ENCODER_CHANNEL_T ch):
	pidController(&input, &output, &setpoint, PID_KP, PID_KI, PID_KD, DIRECT) {	
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
	for(int i = SPEED_INPUT_DATA_LENGTH - 1; i > 0; --i){
		inputData[i] = inputData[i - 1];
	}
	inputData[0] = calculateInputSpeed(Encoder_GetCount(encoderCh));
	Encoder_ResetCount(encoderCh);
}

/*
 * @brief:	cconvert percentage to dutyCycle
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
	static double setLeft, setRight;

	setLeft = (linearSpeed + angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS; 		// Velocidad angular de las ruedas
	setRight = (linearSpeed - angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS;

	leftMotor.setpoint = setLeft > MAX_ANGULAR_SPEED ? MAX_ANGULAR_SPEED : setLeft;
	rightMotor.setpoint = setRight > MAX_ANGULAR_SPEED ? MAX_ANGULAR_SPEED : setRight;
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
		movementModule.leftMotor.getSpeed();
		movementModule.rightMotor.getSpeed();
		movementModule.leftMotor.input = filter(movementModule.leftMotor.inputData);
		movementModule.rightMotor.input = filter(movementModule.rightMotor.inputData);
		
		movementModule.rightMotor.pidController.Compute();
		movementModule.leftMotor.pidController.Compute();

		movementModule.leftMotor.setSpeed();
		movementModule.rightMotor.setSpeed();
		vTaskDelay(xDelay50ms);
		//vTaskDelayUntil( &xLastWakeTime, xDelay50ms );
	}
}

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
double calculateInputSpeed(uint32_t value)
{
	double speed = (double) value/(ENCODER_STEPS_PER_REVOLUTION * CONTROL_SAMPLE_PERIOD_MS * 0.001 * REDUCTION_FACTOR)*2.0*3.1415; // Velocidad angular del eje de las ruedas
	
	return (speed >= 2*MAX_ANGULAR_SPEED) ? MAX_ANGULAR_SPEED : speed; // Esto es para filtrar el ruido de medida (mediciones absurdas del encoder las saturamos)
}

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
double filter(double inputData[])
{
	static double ret;
	ret = 0;
	for(int i = 0; i < FILTER_ORDER; ++i){
//		ret +=  coeffs[i]*inputData[i];
		ret +=  inputData[i];
	}
	ret = ret/((double)FILTER_ORDER);
	return ret;
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
	configASSERT(xTaskCreate( mcmMainTask, "MC Main task", 100	, NULL, 1, NULL ) == pdTRUE);
}

/*
 * @brief:	Sets the linear speed for the vehicle.
 * @param:	v:   linear speed, as a double the sign defines the direction
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setLinearSpeed(double v)
{
	movementModule.setLinerSpeed(v);	// Recibe unvalor entre 0 y 1, tenemos que multiplicar por setpoint maximo
	movementModule.calculateSetpoints();
}

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setAngularSpeed(double w)
{
	movementModule.setAngularSpeed(w);
	movementModule.calculateSetpoints();
}

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_getWheelSpeeds(double * speeds)
{
	speeds[0] = movementModule.leftMotor.setpoint;
	speeds[1] = movementModule.leftMotor.input;
	speeds[2] = movementModule.rightMotor.setpoint;
	speeds[3] = movementModule.rightMotor.input;
}

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setPIDTunings(double Kp, double Ki, double Kd)
{
	movementModule.leftMotor.pidController.SetTunings(Kp, Ki, Kd, 1);
	movementModule.rightMotor.pidController.SetTunings(Kp, Ki, Kd, 1);
}

/*==================[end of file]============================================*/
