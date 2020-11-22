/* Copyright 2015-2016, Eric Pernia.
 * All rights reserved.
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Date: 2015-09-23 */

/*==================[inclusions]=============================================*/

#include "my_sapi_gpio.h"
#include "chip.h"
#include "scu_18xx_43xx.h" 	// USER-DEFINED

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
callBackFuncPtr_t fCallback;
/*==================[internal data definition]===============================*/

const pinInitGpioLpc4337_t gpioPinsInit[] = {

   //{ {PinNamePortN ,PinNamePinN}, PinFUNC, {GpioPortN, GpioPinN} }
   
   // --------------------------------------------------------------- 
   //                             CIAA-NXP                            
   // --------------------------------------------------------------- 
   //       sAPI  Connector  Serigraphy
   // --------------------------------------------------------------- 

      { {4, 0}, FUNC0, {2, 0} },   //  0   DI0     BORN_24   DIN0
      { {4, 1}, FUNC0, {2, 1} },   //  1   DI1     BORN_25   DIN1
      { {4, 2}, FUNC0, {2, 2} },   //  2   DI2     BORN_26   DIN2
      { {4, 3}, FUNC0, {2, 3} },   //  3   DI3     BORN_27   DIN3
      { {7, 3}, FUNC0, {3,11} },   //  4   DI4     BORN_28   DIN4
      { {7, 4}, FUNC0, {3,12} },   //  5   DI5     BORN_29   DIN5
      { {7, 5}, FUNC0, {3,13} },   //  6   DI6     BORN_30   DIN6
      { {7, 6}, FUNC0, {3,14} },   //  7   DI7     BORN_31   DIN7

      { {2, 1}, FUNC4, {5, 1} },   //  8   DO0     BORN_14   DOUT0
      { {4, 6}, FUNC0, {2, 6} },   //  9   DO1     BORN_06   DOUT1
      { {4, 5}, FUNC0, {2, 5} },   // 10   DO2     BORN_08   DOUT2
      { {4, 4}, FUNC0, {2, 4} },   // 11   DO3     BORN_10   DOUT3
      { {4, 8}, FUNC4, {5,12} },   // 12   DO4     BORN_14   DOUT4
      { {4, 9}, FUNC4, {5,13} },   // 13   DO5     BORN_15   DOUT5
      { {4,10}, FUNC4, {5,14} },   // 14   DO6     BORN_16   DOUT6
      { {1, 5}, FUNC0, {1, 8} },   // 15   DO7     BORN_17   DOUT7

      // P12
      { {6, 1}, FUNC0, {3, 0} },   // 16   GPIO0   P12
      { {2, 5}, FUNC4, {5, 5} },   // 17   GPIO1   P12
      { {7, 0}, FUNC0, {3, 8} },   // 18   GPIO2   P12
      { {6, 7}, FUNC4, {5,15} },   // 19   GPIO3   P12
      { {6, 3}, FUNC0, {3, 2} },   // 20   GPIO7   P12
      { {6, 6}, FUNC0, {0, 5} },   // 21   GPIO8   P12
   
      // P14 header
      { {1, 3}, FUNC0, {0,10} },   // 16   SPI_MISO P14
      { {1, 4}, FUNC0, {0,11} },   // 17   SPI_MOSI P14
      { {6, 7}, FUNC4, {5,15} },   // 18   SPI_CS   P14

      //#error CIAA-NXP
};

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void gpioObtainPinInit( gpioMap_t pin,
                               int8_t *pinNamePort, int8_t *pinNamePin,
                               int8_t *func, int8_t *gpioPort,
                               int8_t *gpioPin )
{

   *pinNamePort = gpioPinsInit[pin].pinName.port;
   *pinNamePin  = gpioPinsInit[pin].pinName.pin;
   *func        = gpioPinsInit[pin].func;
   *gpioPort    = gpioPinsInit[pin].gpio.port;
   *gpioPin     = gpioPinsInit[pin].gpio.pin;
}

/*==================[external functions definition]==========================*/

bool_t gpioInit( gpioMap_t pin, gpioInit_t config )
{
   if( pin == VCC ){
	  return FALSE;
   }
   if( pin == GND ){
	  return FALSE;
   }

   bool_t ret_val     = 1;

   int8_t pinNamePort = 0;
   int8_t pinNamePin  = 0;

   int8_t func        = 0;

   int8_t gpioPort    = 0;
   int8_t gpioPin     = 0;

   gpioObtainPinInit( pin, &pinNamePort, &pinNamePin, &func,
                      &gpioPort, &gpioPin );

   switch(config) {

   case GPIO_ENABLE:
      /* Initializes GPIO */
      Chip_GPIO_Init(LPC_GPIO_PORT);
      break;

   case GPIO_INPUT:
      Chip_SCU_PinMux(
         pinNamePort,
         pinNamePin,
         SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS,
         func
      );
      Chip_GPIO_SetDir( LPC_GPIO_PORT, gpioPort, ( 1 << gpioPin ), GPIO_INPUT );
      break;

   case GPIO_INPUT_PULLUP:
      Chip_SCU_PinMux(
         pinNamePort,
         pinNamePin,
         SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS,
         func
      );
      Chip_GPIO_SetDir( LPC_GPIO_PORT, gpioPort, ( 1 << gpioPin ), GPIO_INPUT );
      break;

   case GPIO_INPUT_PULLDOWN:
      Chip_SCU_PinMux(
         pinNamePort,
         pinNamePin,
         SCU_MODE_PULLDOWN | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS,
         func
      );
      Chip_GPIO_SetDir( LPC_GPIO_PORT, gpioPort, ( 1 << gpioPin ), GPIO_INPUT );
      break;
   case GPIO_INPUT_PULLUP_PULLDOWN:
      Chip_SCU_PinMux(
         pinNamePort,
         pinNamePin,
         SCU_MODE_REPEATER | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS,
         func
      );
      Chip_GPIO_SetDir( LPC_GPIO_PORT, gpioPort, ( 1 << gpioPin ), GPIO_INPUT );
      break;

   case GPIO_OUTPUT:
      Chip_SCU_PinMux(
         pinNamePort,
         pinNamePin,
         SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN,
         func
      );
      Chip_GPIO_SetDir( LPC_GPIO_PORT, gpioPort, ( 1 << gpioPin ), GPIO_OUTPUT );
      Chip_GPIO_SetPinState( LPC_GPIO_PORT, gpioPort, gpioPin, 0);
      break;

   default:
      ret_val = 0;
      break;
   }

   return ret_val;

}


bool_t gpioWrite( gpioMap_t pin, bool_t value )
{
   if( pin == VCC ){
	  return FALSE;
   }
   if( pin == GND ){
	  return FALSE;
   }

   bool_t ret_val     = 1;

   int8_t pinNamePort = 0;
   int8_t pinNamePin  = 0;

   int8_t func        = 0;

   int8_t gpioPort    = 0;
   int8_t gpioPin     = 0;

   gpioObtainPinInit( pin, &pinNamePort, &pinNamePin, &func,
                      &gpioPort, &gpioPin );

   Chip_GPIO_SetPinState( LPC_GPIO_PORT, gpioPort, gpioPin, value);

   return ret_val;
}


bool_t gpioToggle( gpioMap_t pin )
{
   return gpioWrite( pin, !gpioRead(pin) );
}


bool_t gpioRead( gpioMap_t pin )
{
   if( pin == VCC ){
      return TRUE;
   }
   if( pin == GND ){
      return FALSE;
   }

   bool_t ret_val     = OFF;

   int8_t pinNamePort = 0;
   int8_t pinNamePin  = 0;

   int8_t func        = 0;

   int8_t gpioPort    = 0;
   int8_t gpioPin     = 0;

   gpioObtainPinInit( pin, &pinNamePort, &pinNamePin, &func,
                      &gpioPort, &gpioPin );

   ret_val = (bool_t) Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, gpioPort, gpioPin );

   return ret_val;
}


void gpioSetInt( gpioMap_t pin, callBackFuncPtr_t f )
{

	int8_t pinNamePort = 0;
	int8_t pinNamePin  = 0;
	int8_t func        = 0;
	int8_t gpioPort    = 0;
	int8_t gpioPin     = 0;
	gpioObtainPinInit( pin, &pinNamePort, &pinNamePin, &func,
	                      &gpioPort, &gpioPin );
	Chip_SCU_GPIOIntPinSel(0, gpioPort, gpioPin);
	fCallback=f;
}
/*==================[end of file]============================================*/
void SGPIO_IRQHandler()
{
	if(fCallback!=NULL)
		fCallback(0);
}
