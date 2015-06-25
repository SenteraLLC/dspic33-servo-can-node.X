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
static int64_t UtilPow( int64_t var_in, uint64_t var_mul, uint8_t power );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
int64_t UtilPoly( int64_t  var_in,
                  uint64_t var_mul,
                  int32_t  coeff[], 
                  uint8_t  coeff_len )
{
    int64_t result = 0;
    uint8_t coeff_idx;
    
    for ( coeff_idx = 0;
          coeff_idx < coeff_len;
          coeff_idx++ )
    {
        result += UtilPow( var_in, var_mul, coeff_idx ) * coeff[ coeff_idx ];
    }
    
    return result;
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
static int64_t UtilPow( int64_t var_in, uint64_t var_mul, uint8_t power )
{    
    int64_t result;
    uint8_t pow_idx;
        
    // Treat special case of power = 0.
    if( power == 0 )
    {
        // Return a value of '1'.
        // -> scale = var_mul
        result = 1 * var_mul;
    }
    // power != 0, compute the result by multiplying 'var' by itself for the
    // number of times equal to 'pow'.
    else
    {
        // Stored 1st power value.
        // -> scale = var_mul
        result = var_in;
        
        // Calculated 2nd to nth power value.
        for( pow_idx = 1;
             pow_idx < power;
             pow_idx++ )
        {
            // Perform additional multiplication of input variable.
            // -> scale = 2 * var_mul
            result *= var_in;
            
            // Down-scale result back to input variable multiplier.
            // -> scale = var_mul
            result /= var_mul;
        }
    }
    
    return result;
}