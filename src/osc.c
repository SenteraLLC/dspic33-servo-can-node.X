////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Oscillator (OSC) driver. 
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "osc.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

void OSCInit( void )
{
    // Initialize the oscillator to operate the CPU clock at 40MHz
    // (i.e. 20 MIPS).
    // 
    // M  := PLLDIV + 2
    // N1 := PLLPRE + 2
    // N2 := 2 * ( PLLPOST + 1 )
    //
    // Fosc = Fin    * (  M / ( N1 * N2 ) )
    //      = 10 MHz * ( 32 / (  2 *  4 ) )
    //      = 40 MHz 
    //
    // Note: Because of the input clock source (Fin) and the requirements for
    // the PLL intermediate frequencies (i.e. Fplli & Fsys), the oscillator can
    // be operated in a singe mode (i.e. External Clock w/ PLL).  The PLL is
    // updated to drive to the correct value before checking that the PLL is
    // locked.
    //
    // Note: On warm-reset (i.e. not POR or BOR) the oscillator retains
    // its SFR settings.  Setting of clock registers are done independent
    // of the reset state to make less complex - i.e. on a warm-reset the
    // clock will be set to the same value it already is (no PLL lock time
    // should be needed).
    //
    CLKDIVbits.PLLPRE  =  0;         // PLL input divided by 2.      (N1)
    CLKDIVbits.PLLPOST =  1;         // PLL output divided by 4.     (N2)
    PLLFBDbits.PLLDIV  = 30;         // PLL feedback divisor is 32.  (M)
    
    // Wait for the PLL to lock.
    while( OSCCONbits.LOCK == 0 );   
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************