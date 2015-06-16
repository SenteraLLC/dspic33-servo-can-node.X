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
static long double UtilPow( uint16_t var, uint8_t pow );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
int16_t UtilPolyMul( uint16_t var, int32_t coeff[], uint8_t coeff_len )
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
static long double UtilPow( uint16_t var, uint8_t pow )
{
    long double result;
    
    result = powl( (long double) var, (long double) pow );
    
    return result;
}