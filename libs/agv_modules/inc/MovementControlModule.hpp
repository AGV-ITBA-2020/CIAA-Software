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
#include "Encoder.h"

using namespace pid;
// using namespace std;

/*==================[macros and definitions]=================================*/
#define FILTER_ORDER 10

#define AGV_AXIS_LONGITUDE 0.5
#define AGV_WHEEL_DIAMETER 0.25
#define AGV_WHEEL_RADIUS (AGV_WHEEL_DIAMETER/2.0)
#define PID_KP 5.0
#define PID_KI 0.0
#define PID_KD 0.0
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

        PID pidController;
        gpioMap_t in1, in2;
        uint8_t enableSCTOut;
        ENCODER_CHANNEL_T encoderCh;
        double input, output, setpoint;
        double inputData[SPEED_INPUT_DATA_LENGTH];

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

#endif /* _MC */

/**
 * @}
 */
