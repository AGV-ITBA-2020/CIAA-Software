/*
 * assert.h
 *
 *  Created on: 26 jul. 2020
 *      Author: mdevo
 */

#ifndef ASSERT_H_
#define ASSERT_H_
#include "stdint.h"

#define assert( x )   if( ( x ) == 0 ) AssertCalled( __FILE__, __LINE__ )

void AssertCalled( const char *pcFile, uint32_t ulLine );

#endif /* ASSERT_H_ */
