/* Copyright 2020, AGV ITBA - 2020
 * 
 * Placeholder
 */

/* Date: 2020-09-12 */

/* 
 * Placeholder
 * 
 */


#ifndef _MC
#define _MC

/*==================[inclusions]=============================================*/
#include "PID_v1.hpp"
// #include "Encoder.h"
#include "EncoderV2.h"
#include "arm_math.h"

using namespace pid;
// using namespace std;

/*==================[macros and definitions]=================================*/
#define BLOCK_SIZE          1
#define FILTER_ORDER        21

#define MAX_WHEEL_ANGULAR_SPEED   5.4 // rad/s Midiendo la salida del eje de las ruedas con la fuente de 12 V sobre el motor DERECHO
#define AGV_MAX_ANGULAR_SPEED  (MAX_WHEEL_ANGULAR_SPEED*AGV_WHEEL_RADIUS/AGV_AXIS_LONGITUDE)
#define AGV_AXIS_LONGITUDE 0.5
#define AGV_WHEEL_DIAMETER 0.25
#define AGV_WHEEL_RADIUS (AGV_WHEEL_DIAMETER/2.0)
#define SPEED_INPUT_DATA_LENGTH FILTER_ORDER


class MotorController_t{
    public:
        MotorController_t(gpioMap_t in1, gpioMap_t in2, uint8_t enableSCTOut, ENCODER_CHANNEL_T ch);

        void init(void);
        void setMotorDutyCtcle(uint8_t value);
        void setMotorDirection(bool_t direction);
        uint8_t outputDutyToPeripheralPWM(double duty);
        void getSpeed(void);
        void setSpeed(void);
        void filterInput(void);
        void Compute(void);

        PID pidController;
        gpioMap_t in1, in2;
        uint8_t enableSCTOut;
        ENCODER_CHANNEL_T encoderCh;
        double input, output, setpoint;

        /* -------------------------------------------------------------------
        * Declare State buffer of size (numTaps + blockSize - 1)
        * ------------------------------------------------------------------- */
        float32_t firStateF32[BLOCK_SIZE + FILTER_ORDER - 1];
        // float32_t input, inputData[SPEED_INPUT_DATA_LENGTH];
        float32_t inputFiltered, inputData;
        arm_fir_instance_f32 S;

    private:
};

class AGVMovementModule_t {
    public:
        AGVMovementModule_t();
        void calculateSetpoints();    
        void setLinerSpeed(double v){ linearSpeed = v; };
        void setAngularSpeed(double w){ angularSpeed = w; };

        MotorController_t leftMotor, rightMotor;
    
    private:
        double linearSpeed, angularSpeed;
};

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/*
 * @brief:	Initialize the MC module.
 * @param:	Placeholder
 * @note:	The MC is in charge of controlling the speed and direction of the vehicle.
 */
void MC_Init(void);


/*
 * @brief:	Sets the linear speed for the vehicle.
 * @param:	v:   linear speed, as a double the sign defines the direction
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setLinearSpeed(double v);

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setAngularSpeed(double w);

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_getWheelSpeeds(double * speeds);

/*
 * @brief:	Sets the angular speed for the vehicle.
 * @param:	w:   angular speed, as a double the sign defines if is clockwise or anti-clockwise.
 * @note:	This value will be controlled by a PID, so settlement time must be taken into account.
 */
void MC_setPIDTunings(double Kp, double Ki, double Kd);
void MC_setRightPIDTunings(double Kp, double Ki, double Kd);
void MC_setLeftPIDTunings(double Kp, double Ki, double Kd);
/*
 * @brief:	Get PID constants
 * @param:	
 * @note:	
 */
void MC_getLeftPIDTunings(double * Kp, double * Ki, double * Kd);
void MC_getRightPIDTunings(double * Kp, double * Ki, double * Kd);

/*
 * @brief:  
 * @param:  
 * @note:   
 */
void MC_SetFilterState(bool_t state);
bool_t MC_GetFilterState();

#endif /* _MC */

/**
 * @}
 */
