////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief  Source code file for defining hardware operation.   
///
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************
#include <xc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************
#include "hw.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************
static long double UtilPow( int16_t var, uint8_t pow );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
int16_t UtilPolyMul( int16_t var, int32_t coeff[], uint8_t coeff_len )
{
    long double value;
    uint8_t coeff_idx;
    
    for ( coeff_idx  = 0;
          coeff_idx <= coeff_len;
          coeff_idx++ )
    {
        value = UtilPow( var, coeff_idx ) * coeff[ coeff_idx ];
    }
    
    return ( (int16_t) value );
}

// Note: internal scaling by 10 is performed; input parameter 'ms_delay'
// should no exceed ~6000.
void UtilDelay( uint16_t ms_delay )
{
    uint16_t start_time;
    uint16_t psnt_time;
    uint16_t p1ms_delay;
    
    // Scale delay time to resolution of Timer1 (i.e. 1ms -> 0.1ms).
    p1ms_delay = ms_delay * 10;
    
    // Initialize timers for identifying delay time.
    start_time = HwTMRp1msGet();
    psnt_time  = start_time;
    
    // Wait for the delay time to elapse before exiting the function.
    //
    // Note: Condition is 'less than or equal to' to guarantee delay time will
    // elapse.  For example, with input parameter 'ms_delay' = 5, the time
    // elapsed will be 5.0-5.1ms.
    //
    while ( psnt_time - start_time <= p1ms_delay )
    {
        psnt_time = HwTMRp1msGet();
    }
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

// required input parameter 'var' scaling = 1E3.
//
// output parameter scaling = 1E6.
//
static int64_t UtilPow( int16_t var, uint8_t pow )
{
    static const int64_t var_mul    = 1E3;
    static const int64_t calc_mul   = 1E6;
    static const int64_t return_mul = 1E6;
    
    int64_t result = 0;
    uint8_t pow_idx;
    int64_t var_scaled;
        
    // Treat special case of pow = 0.
    if( pow == 0 )
    {
        // Return a value of '1'.
        // -> scale = 1E6
        //
        result = 1 * return_mul;
    }
    // pow != 0, compute the result by multiplying 'var' by itself for the
    // number of times equal to 'pow'.
    else
    {
        // Up-scale the input parameter 'var' for internal calculation.
        // -> scale = 1E3 * ( 1E6 / 1E3 ) = 1E6
        var_scaled = var * ( calc_mul / var_mul );
        
        result = var_scaled;
        
        for( pow_idx = 0;
             pow_idx < ( pow - 1 );
             pow_idx++ )
        {
            // Perform additional multiplication of 'var' value.
            // -> scale = 1E6 * 1E6 = 1E12
            result *= var_scaled;
            
            // Down-scale result back to calculation multiplier.
            // -> scale = 1E12 / 1E6 = 1E6
            result /= calc_mul;
        }
        
        // Down-scale calculation result to output scaling.
        //  -> scale = 1E6 / ( 1E6 / 1E6 ) = 1E6
        result = result / ( calc_mul / return_mul );
    }
    
    return result;
}