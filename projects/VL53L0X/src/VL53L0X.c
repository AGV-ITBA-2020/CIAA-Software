/*
 * VL53L0X.c
 *
 *  Created on: 26 jul. 2020
 *      Author: mdevo
 */

#include "VL53L0X.h"
#include "i2c_18xx_43xx.h"
#include "scu_18xx_43xx.h"

#define VL53L0X_I2C_ADD 0x52

void VL53L0X_Init()
{
	Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);
	Chip_I2C_Init(VL53L0X_I2C_INST);
	Chip_I2C_SetClockRate(VL53L0X_I2C_INST, VL53L0X_I2C_CLK);	// Clock = 200kHz
	Chip_I2C_SetMasterEventHandler( VL53L0X_I2C_INST, Chip_I2C_EventHandlerPolling );
	// No need for mux setup.
}

int VL53L0X_TransmitTest(uint8_t * x, uint8_t length)
{
	return Chip_I2C_MasterSend(VL53L0X_I2C_INST, VL53L0X_I2C_ADD, x, length);
}

int VL53L0X_ReadRegisters(uint8_t regIndex, int len, uint8_t * buff)
{
	//int bytesRx = 0;
	//return Chip_I2C_MasterCmdRead(VL53L0X_I2C_INST, VL53L0X_I2C_ADD, regIndex, buff, len);
	/*if(bytesRx == len)
		return true;
	else
		return false;*/

	I2CM_XFER_T i2cData;

	i2cData.slaveAddr = VL53L0X_I2C_ADD;
	i2cData.options   = 0;
	i2cData.status    = 0;
	i2cData.txBuff    = &regIndex;
	i2cData.txSz      = 1;
	i2cData.rxBuff    = buff;
	i2cData.rxSz      = len;

	if( Chip_I2CM_XferBlocking( LPC_I2C0, &i2cData ) == 0 )
		return FALSE;

	return TRUE;
}
