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

#define MAX_DUTY_CYCLE		100.0
#define MAX_SCT_DUTY_CYCLE	255.0
#define MIN_SCT_DUTY_CYCLE 153.0		// Minimo duty cycle al que se empiezan a mover los motores (es un 60% de duty y sin carga)
#define TRUNCATE_SCT_DUTY_CYCLE 160.0	// El changui adicional para la funcion partida
#define DC_TO_SCT_MAPPING_ALPHA ((MAX_SCT_DUTY_CYCLE-MIN_SCT_DUTY_CYCLE)/MAX_DUTY_CYCLE)
#define REDUCTION_FACTOR	60.06

#define CONTROL_SAMPLE_PERIOD_MS 50.0
// Los valores medios que conseguimos son Kp=3 Ki=5 Kd=0
#define PID_KP 5
#define PID_KI 15
#define PID_KD 3

#define abs(x)  ( (x<0) ? -(x) : x )

const float32_t firCoeffs32[ FILTER_ORDER ] =
{
	// Filtro minimax orden 20:
	0.0182,    0.0041,   -0.0176,   -0.0436,   -0.0555,   -0.0407,    0.0037,    0.0729,    0.1532,    0.2201,
	0.2465,    0.2201,    0.1532,    0.0729,    0.0037,   -0.0407,   -0.0555,   -0.0436,   -0.0176,    0.0041,
	0.0182

	// Filtro messi orden 10
//	0.0035, -0.0214, 0.0630, -0.1249, 0.1834, 0.7923, 0.1834, -0.1249, 0.0630, -0.0214
		// 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1
};

/*==================[internal data declaration]==============================*/
AGVMovementModule_t movementModule;
bool_t useFilter = false;

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
// double calculateInputSpeed(uint32_t value);
double calculateInputSpeed(double timeBetweenSteps);

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

	// Encoder_Init(encoderCh);
	EncoderV2_Init(encoderCh);

	// Turn the PID on
  	pidController.SetMode(AUTOMATIC);	
  	pidController.SetOutputLimits(0, MAX_DUTY_CYCLE);
	pidController.SetSampleTime(CONTROL_SAMPLE_PERIOD_MS);

	/* Call FIR init function to initialize the instance structure. */
  	arm_fir_init_f32(&S, FILTER_ORDER, (float32_t *)&firCoeffs32[0], &firStateF32[0], BLOCK_SIZE);
}

/*
 * @brief:	Initialize the MC module
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void MotorController_t::setMotorDutyCycle(uint8_t value)
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
	// for(int i = SPEED_INPUT_DATA_LENGTH - 1; i > 0; --i){
	// 	inputData[i] = inputData[i - 1];
	// }
	// inputData = (float32_t)calculateInputSpeed(Encoder_GetCount(encoderCh));
	inputData = (float32_t)calculateInputSpeed(COUNT_TO_SECS(EncoderV2_GetCountMedian(encoderCh)));
	// Encoder_ResetCount(encoderCh);
	EncoderV2_ResetCount(encoderCh);
}

/*
 * @brief:	cconvert percentage to dutyCycle
 * @param:	w es la velocidad angular medida en la rueda
 * @note:	Converts RPM speed to dutyCycle value.
 */
void MotorController_t::setSpeed(void)
{
	uint8_t sctDuty = DC_TO_SCT_MAPPING_ALPHA * output + MIN_SCT_DUTY_CYCLE;
	if((sctDuty <= TRUNCATE_SCT_DUTY_CYCLE) && ((setpoint == 0) || ((setpoint - input) <= 0)))
	{
		sctDuty = 0;
	}

	if(sctDuty < 0){
		setMotorDirection(true);
	}else{
		setMotorDirection(false);
	}
	setMotorDutyCycle(abs(sctDuty));
}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
void MotorController_t::filterInput(void)
{
	if(useFilter)
		arm_fir_f32(&S, &inputData, &inputFiltered, BLOCK_SIZE);
	input = useFilter ? (double)inputFiltered : (double)inputData;
}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
void MotorController_t::Compute(void)
{
	if(setpoint == 0)	// Si llega unsetpoint 0, queremos pasar a modo manual para controlar el ouput y ponerlo en 0
	{
		pidController.SetMode(MANUAL);
		output = 0;
	}
	else if(!pidController.Compute()){	// Sino tratamos de computar/computamos el PID
		pidController.SetMode(AUTOMATIC);	// Si todav[ia esta en modo manual, lo pasamos a modo automatico y computamos
		pidController.Compute();
	}
}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
AGVMovementModule_t::AGVMovementModule_t(void): 
leftMotor(GPIO2, GPIO3, LEFT_MOTOR_OUTPUT, ENCODER_LEFT), 
rightMotor(GPIO7, GPIO8, RIGHT_MOTOR_OUTPUT, ENCODER_RIGHT) {

	rightMotor.pidController.SetTunings(PID_KP, PID_KI, PID_KD*0.66, 1);	// El motor derecho es más sensible a kd (se lo bajo un poco)
}

/*
 * @brief:	calculateSpeeds output values
 * @param:	Placeholder
 * @note:	Is just a testing function for now.
 */
void AGVMovementModule_t::calculateSetpoints(void)
{
	static double setLeft, setRight;

	setLeft = angularSpeed*AGV_AXIS_LONGITUDE/AGV_WHEEL_RADIUS; 		// Velocidad angular de las ruedas
	setRight = -angularSpeed*AGV_AXIS_LONGITUDE/AGV_WHEEL_RADIUS;

	double linealVelDelta = MAX_WHEEL_ANGULAR_SPEED - (setLeft >= 0 ? setLeft : setRight);	// Get absolute of angular velocity of wheel
	if(linearSpeed > linealVelDelta)
		linearSpeed = linealVelDelta;

	setLeft += linearSpeed/AGV_WHEEL_RADIUS;	// Velocidad angular final de las ruedas
	setRight += linearSpeed/AGV_WHEEL_RADIUS;

	setLeft = setLeft < 0 ? 0 : setLeft;
	setRight = setRight < 0 ? 0 : setRight;
	// setLeft = (linearSpeed + angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS; 		
	// setRight = (linearSpeed - angularSpeed*AGV_AXIS_LONGITUDE)/AGV_WHEEL_RADIUS;

	leftMotor.setpoint = setLeft > MAX_WHEEL_ANGULAR_SPEED ? MAX_WHEEL_ANGULAR_SPEED : setLeft;
	rightMotor.setpoint = setRight > MAX_WHEEL_ANGULAR_SPEED ? MAX_WHEEL_ANGULAR_SPEED : setRight;
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
		// movementModule.leftMotor.input = filter(movementModule.leftMotor.inputData);
		// movementModule.rightMotor.input = filter(movementModule.rightMotor.inputData);
		movementModule.leftMotor.filterInput();
		movementModule.rightMotor.filterInput();
		
		movementModule.rightMotor.Compute();
		movementModule.leftMotor.Compute();



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
// double calculateInputSpeed(uint32_t value)
double calculateInputSpeed(double timeBetweenSteps)
{
	// double speed = (double) value/(ENCODER_STEPS_PER_REVOLUTION * CONTROL_SAMPLE_PERIOD_MS * 0.001 * REDUCTION_FACTOR)*2.0*3.1415; // Velocidad angular del eje de las ruedas
	double speed = 1.0/(ENCODER_STEPS_PER_REVOLUTION * timeBetweenSteps * REDUCTION_FACTOR)*2.0*3.1415; // Velocidad angular del eje de las ruedas
	
	return (speed >= 2*MAX_WHEEL_ANGULAR_SPEED) ? MAX_WHEEL_ANGULAR_SPEED : speed; // Esto es para filtrar el ruido de medida (mediciones absurdas del encoder las saturamos)
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
void MC_setRightPIDTunings(double Kp, double Ki, double Kd)
{
	movementModule.rightMotor.pidController.SetTunings(Kp, Ki, Kd, 1);
}
void MC_setLeftPIDTunings(double Kp, double Ki, double Kd)
{
	movementModule.leftMotor.pidController.SetTunings(Kp, Ki, Kd, 1);
}

void MC_getRightPIDTunings(double * Kp, double * Ki, double * Kd)
{
	*Kp = movementModule.rightMotor.pidController.GetKp();
	*Ki = movementModule.rightMotor.pidController.GetKi();
	*Kd = movementModule.rightMotor.pidController.GetKd();
}

void MC_getLeftPIDTunings(double * Kp, double * Ki, double * Kd)
{
	*Kp = movementModule.leftMotor.pidController.GetKp();
	*Ki = movementModule.leftMotor.pidController.GetKi();
	*Kd = movementModule.leftMotor.pidController.GetKd();
}

void MC_SetFilterState(bool_t state)
{
	useFilter = state;
}
bool_t MC_GetFilterState()
{
	return useFilter;
}

/*==================[end of file]============================================*/
