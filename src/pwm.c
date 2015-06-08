
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

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************
#include "pwm.h"

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

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
void PWMInit ( void )
{
    PTCONbits.PTEN = 0; // Disable the PWM module.
    
    // Fosc = 40MHz
    // Fpwm = 50Hz
    //
    // Fcnt := PWM counter frequency (i.e. the resolution of the PWM).
    // 
    // Fcnt = Fosc  / PCLKDIV
    //      = 40MHz / 20
    //      = 2MHz
    //
    // PTPER = Fcnt / Fpwm
    //       = 2MHz / 50Hz
    //       = 40000
    //
    //
    //
    //
    
    
    PTCON2bits.PCLKDIV = 20; // Set PWM prescaler
    
    
    // PWM3 I/O Control Register
    // 
    //  bits    15: PENH   =  0 - PWM3H pin controlled by GPIO module.
    //  bits    14: PENL   =  1 - PWM3L pin controlled by PWM3 module.
    //  bits    13: POLH   =  0 - N/A, b/c PENH = 0.
    //  bits    12: POLL   =  0 - PWM3L pin is active-high.
    //  bits 11-10: PMOD   = 01 - Redundant output mode.
    //  bits     9: OVRENH =  0 - N/A, b/c PENH = 0.
    //  bits     8: OVRENL =  0 - Override disabled, PWM3 generator sets PWM3L.
    //  bits  7- 6: OVRDAT = 00 - N/A, b/c OVRENL = 0.
    //  bits  5- 4: FLTDAT = 00 - N/A, b/c FLTMOD = 0.
    //  bits  3- 2: CLDAT  = 00 - N/A, b/c CLMOD = 0.
    //  bits     1: SWAP   =  0 - No swap, pints PWM3H/L mapped to respective pins.
    //  bits     0: OSYNC  =  0 - N/A, b/c OVRENL = 0.
    //
    // Note: modification of the IOCON3 register is write protected.  A unlock
    // sequence is required before writing the register in which the '0xABCD' 
    // and '0x1234' keys must be written to the PWMKEY register before IOCON3 
    // can be written.  Writing of the IOCON3 must be the next SFR access after
    // the unlock sequence.
    //
    PWMKEY = 0xABCD;
    PWMKEY = 0x1234;
    IOCON3 = 0x4400;
    
}


// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
