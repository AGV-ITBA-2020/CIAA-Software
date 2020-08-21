/*
 * assert.c
 *
 *  Created on: 26 jul. 2020
 *      Author: mdevo
 */

#include "assert.h"
#include "stdio.h"

void AssertCalled( const char *pcFile, uint32_t ulLine )
{
	/* Inside this function, pcFile holds the name of the source file that contains
	the line that detected the error, and ulLine holds the line number in the source
	file. The pcFile and ulLine values can be printed out, or otherwise recorded,
	before the following infinite loop is entered. */
#ifdef INC_FREERTOS_H
	taskENTER_CRITICAL();
	{
#endif
		printf("\r\nASSERT>> Ln=%d ; File=%s \r\n", ulLine, pcFile);
	    fflush( stdout );
#ifdef INC_FREERTOS_H
	}
	taskEXIT_CRITICAL();
	/* Disable interrupts so the tick interrupt stops executing, then sit in a loop
	so execution does not move past the line that failed the assertion. */
	taskDISABLE_INTERRUPTS();
#endif
	for( ;; );
}
