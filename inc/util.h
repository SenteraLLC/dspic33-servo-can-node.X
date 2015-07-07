////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Utility functions.
////////////////////////////////////////////////////////////////////////////////

#ifndef UTIL_H_
#define	UTIL_H_

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

#include <xc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Declarations *************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  Polynomial calculation of 32-bit variable and coefficients.
///
/// @param  var_in
///             The input variable.
/// @param  calc_qnum
///             The Q-number (i.e. base-2 radix point) of the input variable.
/// @param  coeff
///             Array of coefficient terms.
/// @param  coeff_len
///             Number of coefficient terms.
///
/// @return The result of the polynomial computation.  Result is scaled by
///         same value as input 'coeff'.
///
/// @note   Calculation can take considerable time depending on the degree of 
///         the polynomial.  Fore example, the time required for computation 
///         of 5th degree polynomial is approximately 0.5ms.
////////////////////////////////////////////////////////////////////////////////
int32_t UtilPoly32( int32_t var_in,
                    uint8_t calc_qnum,
                    int32_t coeff[], 
                    uint8_t coeff_len );

////////////////////////////////////////////////////////////////////////////////
/// @brief  Delay specified amount of time.
///
/// @param  ms_time
///             Number of milliseconds to delay.
///
/// @note   Internal scaling by 10 is performed; therefore, input parameter 
///         should not exceed 6553.
///
/// @note   Resolution of 0.1ms is achieved.  Therefore the time delayed by
///         the function [ ms_time : ms_time+0.1 ].
////////////////////////////////////////////////////////////////////////////////
void UtilDelay( uint16_t ms_time );

#endif	// UTIL_H_