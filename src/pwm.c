////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Pulse Width Modulation (PWM) driver.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "pwm.h"

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

void PWMInit ( void )
{
    // Disable the PWM module.
    //
    // Note: Should already be disabled out of reset; setting is included for
    // robustness.
    //
    PTCONbits.PTEN = 0;
    
    PTCONbits.PTSIDL    = 0;    // N/A, b/c CPU idle mode not used.
    PTCONbits.SEIEN     = 0;    // Special event interrupt is disabled.
    PTCONbits.EIPU      = 0;    // N/A, b/c ITB = 1.
    PTCONbits.SYNCPOL   = 0;    // N/A, External synchronization not used.
    PTCONbits.SYNCOEN   = 0;    // SYNCO output is disabled.
    PTCONbits.SYNCEN    = 0;    // External synchronization of time base disabled.
    PTCONbits.SYNCSRC   = 0;    // N/A, b/c external synchronization not used.
    PTCONbits.SEVTPS    = 0;    // N/A, b/c special event trigger not used.
    
    // Fosc = 40MHz
    // Fpwm = 50Hz
    //
    // Fcnt := PWM counter frequency (i.e. resolution of the PWM).
    // 
    // Fcnt = Fosc  / PCLKDIV
    //      = 40MHz / 16
    //      = 2.5MHz
    //
    // PHASE3 = Fcnt   / Fpwm
    //        = 2.5MHz / 50Hz
    //        = 50000
    //
    // PDC3 = PHASE3 * Fpwm * 1.5ms
    //      = 50000  * 50Hz * 1.5ms
    //      = 3750
    //
    // Note: A PWM prescaler is chosen which gives the greatest PWM resolution
    // without overflowing the 16-bit period selection register 'PHASE3'.
    //
    // Note: An initial PWM duty cycle is selected for the neutral (i.e. 1.5ms)
    // position.
    //
    PTCON2bits.PCLKDIV  = 0b100;    // Select the PWM perscaler (0b100 = 16 div).
    PHASE3              = 50000;    // Select the PWM period.
    PDC3                = 3750;     // Select the PWM duty cycle.
    
    CHOPbits.CHPCLKEN   = 0;    // Chop clock generator is disabled.
    
    PWMCON3bits.FLTIEN  = 0;    // Fault interrupts disabled.
    PWMCON3bits.CLIEN   = 0;    // Current-limit interrupt disabled.
    PWMCON3bits.TRGIEN  = 0;    // Trigger interrupt disabled.
    PWMCON3bits.ITB     = 1;    // Select independent time base mode; PWM3 period set with register 'PHASE3'.
    PWMCON3bits.MDCS    = 0;    // Select independent duty cycle; PWM3 duty cycle set with register 'PDC3'.
    PWMCON3bits.DTC     = 0b10; // Dead time function is disabled.
    PWMCON3bits.DTCP    = 0;    // N/A, b/c DTC = 0b10.
    PWMCON3bits.CAM     = 0;    // Edge-aligned mode is enabled (i.e. not center-aligned mode).
    PWMCON3bits.XPRES   = 0;    // External pins do not affect PWM3 time base.
    PWMCON3bits.IUE     = 0;    // Updates to the period (PHASE3) and duty cycle (PDC3) registers are synchronized to the PMW3 period boundary.
    
    // PWM3 I/O Control Register
    // 
    //  bits    15: PENH   =  1 - PWM3H pin controlled by PWM3 module.
    //  bits    14: PENL   =  0 - PWM3L pin controlled by GPIO module.
    //  bits    13: POLH   =  0 - PWM3H pin is active-high.
    //  bits    12: POLL   =  0 - N/A, b/c PENL = 0.
    //  bits 11-10: PMOD   = 01 - Redundant output mode.
    //  bits     9: OVRENH =  0 - N/A, b/c PENH = 0.
    //  bits     8: OVRENL =  0 - Override disabled, PWM3 generator sets PWM3L.
    //  bits  7- 6: OVRDAT = 00 - N/A, b/c OVRENL = 0.
    //  bits  5- 4: FLTDAT = 00 - N/A, b/c FLTMOD = 0.
    //  bits  3- 2: CLDAT  = 00 - N/A, b/c CLMOD = 0.
    //  bits     1: SWAP   =  0 - No swap, pins PWM3H/L mapped to respective pins.
    //  bits     0: OSYNC  =  0 - N/A, b/c OVRENL = 0.
    //
    // Note: Modification of the IOCON3 register is write protected.  An unlock
    // sequence is required before writing the register in which the '0xABCD' 
    // and '0x4321' keys must be written to the PWMKEY register before IOCON3 
    // can be written.  Writing of the IOCON3 must be the next SFR access after
    // the unlock sequence.
    //
    PWMKEY = 0xABCD;
    PWMKEY = 0x4321;
    IOCON3 = 0x8400;
    
    // PWM3 Fault Current-Limit Control Register
    //
    // bits 1-0: FLTMOD = 0B11 - Fault input is disabled.
    //
    // Note: Modification of the IOCON3 register is write protected.  An unlock
    // sequence is required before writing the register in which the '0xABCD' 
    // and '0x4321' keys must be written to the PWMKEY register before IOCON3 
    // can be written.  Writing of the IOCON3 must be the next SFR access after
    // the unlock sequence.
    //
    PWMKEY  = 0xABCD;
    PWMKEY  = 0x4321;
    FCLCON3 = 0x0003;
    
    AUXCON3bits.CHOPHEN = 0; // PWM3H chopping function is disabled.
    AUXCON3bits.CHOPLEN = 0; // PWM3L chopping function is disabled.
}

void PWMEnable ( void )
{
    // Enable the PWM operation.
    PTCONbits.PTEN = 1;
}

void PWMDutySet ( uint16_t pwm_duty )
{
    // Update the hardware register setting for the PWM duty cycle.
    //
    // Note: PWM hardware configuration selected for 2.5MHz (0.4us) resolution.
    // Therefore, input parameter (1us LSB) must be scaled by 2.5 for
    // interfacing with hardware configuration.
    //
    // Note: Immediate vs. period-synchronized updating selected in
    // register bit 'PWMCON3.IUE'.
    //
    PDC3 = ( pwm_duty * 5U ) / 2U;
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
