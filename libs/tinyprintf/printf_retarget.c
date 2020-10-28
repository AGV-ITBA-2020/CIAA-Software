#include <unistd.h>
#include "my_sapi_uart.h"
void _putchar(char character)
{
	while(!uartTxReady(UART_USB));
	uartTxWrite(UART_USB, character);
  // write(1, &character, 1);
}
