////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Timer (TMR) driver. 
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "tmr.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

/// Timer2 0.1ms up-counter with roll-over.
///
/// @note   Multi-threaded data services every 0.1ms and accessible through
///         accessor function.
static volatile uint16_t tmr2_p1ms_cnt = 0;

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

static void TMR1Init( void );
static void TMR2Init( void );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

void TMRInit ( void )
{
    TMR1Init();
    TMR2Init();
}

void TMR1Enable ( void )
{
    // Enable timer 1.
    T1CONbits.TON = 1;
}

void TMR1Disable ( void )
{
    // Disable timer 1.
    T1CONbits.TON = 0;
}

void TMR1Service ( void )
{
    // Clear the hardware interrupt flag.
    IFS0bits.T1IF = 0; 
}

void TMR2Service ( void )
{
    // Clear the hardware interrupt flag.
    IFS0bits.T2IF = 0; 
    
    // Increment the counter (16-bit roll-over counter).
    tmr2_p1ms_cnt++;
}

uint16_t TMR2p1msGet ( void )
{
    return tmr2_p1ms_cnt;
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initialize Timer 1 hardware configuration.
////////////////////////////////////////////////////////////////////////////////
static void TMR1Init( void )
{
    // Timer 1 is operated in 'Timer Mode' - the free-running timer is
    // configured to trigger an interrupt ever 10ms.  This is performed to
    // drive the software executive 10ms processing thread.
    //
    // Timer 1 is fed by the instruction/peripheral clock (Fp), see
    // datasheet p. 123.
    // 
    // Fp       = Fosc / 2                              
    //          = 20MHz
    //
    // Ft1int   = ( Fp    / Prescale ) / ( PR1   + 1 )
    //          = ( 20Mhz / 8        ) / ( 24999 + 1 )
    //          = 100Hz
    //
    // Note: timer configured (TSIDL) for continuous operation in idle mode.
    // Idle mode is not performed by the CPU; therefore, this setting is purely 
    // for robustness.
    //
    T1CONbits.TON   = 0;        // Disable Timer.
    T1CONbits.TCS   = 0;        // Select internal instruction cycle clock.
    T1CONbits.TGATE = 0;        // Select Timer (i.e. not Gated) mode.
    
    T1CONbits.TSIDL = 0;        // Select continuous operation in idle mode.
    
    T1CONbits.TCKPS = 0b01;     // Select prescale = 8.
    
    TMR1            = 0;        // Clear timer value register.
    PR1             = 24999;    // Set the period value.
    
    IPC0bits.T1IP   = 1;        // Select Timer 1 interrupt priority level.
    IFS0bits.T1IF   = 0;        // Clear Timer 1 interrupt flag.
    IEC0bits.T1IE   = 1;        // Enable Time 1 interrupt.
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initialize Timer 2 hardware configuration.
////////////////////////////////////////////////////////////////////////////////
static void TMR2Init( void )
{
    // Timer 2 is operated in 'Timer Mode' - the free-running timer is
    // configured to provide a 0.1ms counter.
    //
    // Timer 2 is fed by the instruction/peripheral clock (Fp), see
    // datasheet p. 123.
    // 
    // Fp       = Fosc / 2                              
    //          = 20MHz
    //
    // Ft1int   = ( Fp    / Prescale ) / ( PR1 + 1 )
    //          = ( 20Mhz / 8        ) / ( 249 + 1 )
    //          = 10KHz
    //
    // Note: timer configured (TSIDL) for continuous operation in idle mode.
    // Idle mode is not performed by the CPU; therefore, this setting is purely 
    // for robustness.
    //
    T2CONbits.TON   = 0;        // Disable Timer.
    T2CONbits.TCS   = 0;        // Select internal instruction cycle clock.
    T2CONbits.TGATE = 0;        // Select Timer (i.e. not Gated) mode.
    T2CONbits.T32   = 0;        // Select 16-bit timer.
    
    T2CONbits.TSIDL = 0;        // Select continuous operation in idle mode.
    
    T2CONbits.TCKPS = 0b01;     // Select prescale = 8.
    
    TMR2            = 0;        // Clear timer value register.
    PR2             = 249;      // Set the period value.
    
    IPC1bits.T2IP   = 2;        // Select Timer 2 interrupt priority level.
    IFS0bits.T2IF   = 0;        // Clear Timer 2 interrupt flag.
    IEC0bits.T2IE   = 1;        // Enable Time 2 interrupt.
    
    T2CONbits.TON   = 1;        // Enable Timer.
}