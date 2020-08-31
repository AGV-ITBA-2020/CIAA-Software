#include "my_sapi_delay.h"
#include "chip.h"


#ifdef TICK_OVER_RTOS
   #ifdef USE_FREERTOS
      #include <FreeRTOS.h>
      #include <timers.h>
   #endif
#endif


/*==================[macros and definitions]=================================*/

#ifndef TICK_OVER_RTOS
   #define tickerCallback SysTick_Handler
#endif




void tickPowerSet( bool_t power );
tick_t tickRead( void );

// This global variable holds the tick count
static tick_t tickCounter;
tick_t tickRateMS = 1; // Used by delay!!! Default 1ms

// Tick Initialization and rate configuration from 1 to 50 ms
bool_t tickInit( tick_t tickRateMSvalue )
{
   #ifdef USE_FREERTOS
      uartWriteString( UART_USB, "Use of tickInit() in a program with freeRTOS has no effect\r\n" );
      return 0;
   #else
      bool_t ret_val = 1;
      tick_t tickRateHz = 0;
      if( tickRateMSvalue == 0 ) {
         tickPowerSet( OFF );
         ret_val = 0;
      } else {
         if( (tickRateMSvalue >= 1) && (tickRateMSvalue <= 50) ) {
            tickRateMS = tickRateMSvalue;
            // tickRateHz = 1000 => 1000 ticks per second =>  1 ms tick
            // tickRateHz =  200 =>  200 ticks per second =>  5 ms tick
            // tickRateHz =  100 =>  100 ticks per second => 10 ms tick
            // tickRateHz =   20 =>   20 ticks per second => 50 ms tick
            // Init SysTick interrupt, tickRateHz ticks per second
            SysTick_Config( SystemCoreClock * tickRateMSvalue / 1000 );
            // if ( SysTick_Config( CMU_ClockFreqGet(cmuClock_CORE) / tickRateHz) ){
            //    //DEBUG_BREAK;
            //    ret_val = 0;
            // }
            tickPowerSet( ON );
         } else {
            // Error, tickRateMS variable not in range (1 <= tickRateMS <= 50)
            ret_val = 0;
         }
      }
      return ret_val;
   #endif
}


void delay( tick_t duration_ms )
{
   tick_t startTime = tickRead();
   while ( (tick_t)(tickRead() - startTime) < duration_ms/tickRateMS );
}


void tickerCallback( void )   // Before SysTick_Handler
{
   // Increment Tick counters
   tickCounter++;
}

// Enable or disable the peripheral energy and clock
void tickPowerSet( bool_t power )
{
   #ifdef USE_FREERTOS
      uartWriteString( UART_USB, "Use of tickPowerSet() in a program with freeRTOS has no effect\r\n" );
   #else
      if( power ) {
         // Enable SysTick IRQ and SysTick Timer
         SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                         SysTick_CTRL_TICKINT_Msk   |
                         SysTick_CTRL_ENABLE_Msk;
      } else {
         // Disable SysTick IRQ and SysTick Timer
         SysTick->CTRL = 0x0000000;
      }
   #endif
}

// Read Tick Counter
tick_t tickRead( void )
{
   #ifdef USE_FREERTOS
      return xTaskGetTickCount();
   #else
      return tickCounter;
   #endif
}

