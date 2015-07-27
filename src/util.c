////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Utility functions.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "util.h"
#include "tmr.h"

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

static int32_t UtilPow( int32_t var_in, uint8_t calc_qnum, uint8_t power );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

int32_t UtilPoly32( int32_t var_in,
                    uint8_t calc_qnum,
                    int32_t coeff[], 
                    uint8_t coeff_len )
{
    int32_t result = 0;
    int32_t var_term;
    int64_t intermediate_mul;
    uint8_t coeff_idx;
    
    for ( coeff_idx = 0;
          coeff_idx < coeff_len;
          coeff_idx++ )
    {
        // Calculate the variable power value.
        var_term = UtilPow( var_in, calc_qnum, coeff_idx );
        
        // Calculate the polynomial term.
        // ~ 64-bit multiplication performed.
        // ~ scale = calc_qnum * coeff_scale
        intermediate_mul = ((int64_t) var_term) * ((int64_t) coeff[ coeff_idx ]);
        
        // Update the result with the polynomial term.
        // ~ scale = coeff_scale
        result += (int32_t) (intermediate_mul >> calc_qnum);
    }
    
    return result;
}

void UtilDelay( uint16_t ms_delay )
{
    uint16_t start_time;
    uint16_t psnt_time;
    uint16_t p1ms_delay;
    
    // Scale delay time to resolution of Timer1 (i.e. 1ms -> 0.1ms).
    p1ms_delay = ms_delay * 10;
    
    // Initialize timers for identifying delay time.
    start_time = TMR2p1msGet();
    psnt_time  = start_time;
    
    // Wait for the delay time to elapse before exiting the function.
    //
    // Note: Condition is 'less than or equal to' to guarantee delay time will
    // elapse.  For example, with input parameter 'ms_delay' = 5, the time
    // elapsed will be 5.0-5.1ms.
    //
    while ( psnt_time - start_time <= p1ms_delay )
    {
        psnt_time = TMR2p1msGet();
    }
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  Power term calculation of 32-bit variable.
///
/// @param  var_in
///             The input variable.
/// @param  calc_qnum
///             The Q-number (i.e. base-2 radix point) of the input variable.
/// @param  power
///             The power (i.e. exponent) of the calculation.
///
/// @return The result of the power computation.  Result is scaled by
///         same value as input 'var_in'.
////////////////////////////////////////////////////////////////////////////////
static int32_t UtilPow( int32_t var_in, uint8_t calc_qnum, uint8_t power )
{    
    int32_t result;
    int64_t intermediate_mul;
    uint8_t pow_idx;
        
    // Treat special case of power = 0.
    if( power == 0 )
    {
        // Return a value of '1'.
        result = 1L << calc_qnum;
    }
    // power != 0, compute the result by multiplying 'var' by itself for the
    // number of times equal to 'pow'.
    else
    {
        // Stored 1st power value.
        result = var_in;
        
        // Calculated 2nd to nth power value.
        for( pow_idx = 1;
             pow_idx < power;
             pow_idx++ )
        {
            // Perform additional multiplication of input variable.
            // ~ 64-bit multiplication performed.
            // ~ scale = 2 * calc_qnum
            intermediate_mul = ((int64_t) result) * ((int64_t) var_in);
            
            // Down-scale result back to input calculation Q-number.
            // ~ scale = calc_qnum
            result = (int32_t) (intermediate_mul >> calc_qnum);
        }
    }
    
    return result;
}