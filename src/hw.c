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
#include "hw.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// DSPIC33EV256GM102 Configuration Bit Settings

// FSEC
#pragma config BWRP = OFF               // Boot Segment Write-Protect Bit (Boot Segment may be written)
#pragma config BSS = DISABLED           // Boot Segment Code-Protect Level bits (No Protection (other than BWRP))
#pragma config BSS2 = OFF               // Boot Segment Control Bit (No Boot Segment)
#pragma config GWRP = OFF               // General Segment Write-Protect Bit (General Segment may be written)
#pragma config GSS = DISABLED           // General Segment Code-Protect Level bits (No Protection (other than GWRP))
#pragma config CWRP = OFF               // Configuration Segment Write-Protect Bit (Configuration Segment may be written)
#pragma config CSS = DISABLED           // Configuration Segment Code-Protect Level bits (No Protection (other than CWRP))
#pragma config AIVTDIS = DISABLE        // Alternate Interrupt Vector Table Disable Bit  (Disable Alternate Vector Table)

// FBSLIM
//#pragma config BSLIM = 0x1FFF           // Boot Segment Code Flash Page Address Limit Bits (Boot Segment Flash Page Address Limit (0-0x1FFF))

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Initial oscillator Source Selection Bits (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config IESO = OFF               // Two Speed Oscillator Start-Up Bit (Start up device with user selected oscillator source)

// FOSC
#pragma config POSCMD = EC              // Primary Oscillator Mode Select Bits (EC (External Clock) mode)
#pragma config OSCIOFNC = ON            // OSC2 Pin I/O Function Enable Bit (OSC2 is general purpose digital I/O pin)
#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration Bit (Allow Only One reconfiguration)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode Bits (Both Clock Switching and Fail-safe Clock Monitor are disabled)
#pragma config PLLKEN = ON              // PLL Lock Enable Bit (Clock switch to PLL source will wait until the PLL lock signal is valid)

// FWDT - Configuration Register
//
// WDT source is a low-power RC (LPRC) oscillator with nominal frequency 
// (Flprc) of 32kHz.
//
// Twto := Watchdog time-out period
//
// Twto =   Tlprc * WDTPRE * WDTPOST
//      = 31.25us *     32 *      32
//      =    32ms
//
// Note: Flprc accuracy is +-15% (see datasheet); therefore, selection of 32ms 
// nominal time-out period results in a minimum time-out period of ~27ms.
//
#pragma config WDTPOST = PS32    // Select WDT postscaler = 32.
#pragma config WDTPRE  = PR32    // Select SDT prescaler = 32.
#pragma config FWDTEN  = OFF     // Disable WDT operation. WDT enabled in software.
#pragma config WINDIS  = OFF     // Operate WDT in Non-Window Mode.
#pragma config WDTWIN  = WIN25   // N/A - b/c of 'WINDIS' setting.

// FPOR
#pragma config BOREN0 = ON              // Brown Out Reset Detection Bit (BOR is Enabled)

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select Bits (Communicate on PGEC1 and PGED1)

// FDMTINTVL
#pragma config DMTIVTL = 0xFFFF         // Lower 16 Bits of 32 Bit DMT Window Interval (Lower 16 bits of 32 bit DMT window interval (0-0xFFFF))

// FDMTINTVH
#pragma config DMTIVTH = 0xFFFF         // Upper 16 Bits of 32 Bit DMT Window Interval (Upper 16 bits of 32 bit DMT window interval (0-0xFFFF))

// FDMTCNTL
#pragma config DMTCNTL = 0xFFFF         // Lower 16 Bits of 32 Bit DMT Instruction Count Time-Out Value (Lower 16 bits of 32 bit DMT instruction count time-out value (0-0xFFFF))

// FDMTCNTH
#pragma config DMTCNTH = 0xFFFF         // Upper 16 Bits of 32 Bit DMT Instruction Count Time-Out Value (Upper 16 bits of 32 bit DMT instruction count time-out value (0-0xFFFF))

// FDMT
#pragma config DMTEN = DISABLE          // Dead Man Timer Enable Bit (Dead Man Timer is Disabled and can be enabled by software)

// FDEVOPT
#pragma config PWMLOCK = OFF             // PWM Lock Enable Bit (Certain PWM registers may only be written after key sequence)
#pragma config ALTI2C1 = OFF            // Alternate I2C1 Pins Selection Bit (I2C1 mapped to SDA1/SCL1 pins)

// FALTREG
#pragma config CTXT1 = NONE             // Interrupt Priority Level (IPL) Selection Bits For Alternate Working Register Set 1 (Not Assigned)
#pragma config CTXT2 = NONE             // Interrupt Priority Level (IPL) Selection Bits For Alternate Working Register Set 2 (Not Assigned)

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************
static void HwOSCInit  ( void );
static void HwI2C1Init ( void );
static void HwTMR2Init ( void );
static void HwTMR1Init ( void );
static void HwIOInit   ( void );

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************

// Note: multi-threaded data.
//
// Note: 16-bit timer with roll-over
static volatile uint16_t hwtmr_p1ms_cnt = 0;

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

void HwInit( void )
{
    HwOSCInit();
    HwTMR1Init();
    HwTMR2Init();
    HwI2C1Init();
    HwIOInit();
}

void HwTMREnable ( void )
{
    // Enable timer 1.
    T1CONbits.TON = 1;
}

void HwTMRDisable ( void )
{
    // Disable timer 1.
    //
    // Note: The timer counter value is cleared when disabling operation.
    T1CONbits.TON = 0;
}

void HwTMR1Service ( void )
{
    // Clear the hardware interrupt flag.
    IFS0bits.T1IF = 0; 
}

void HwTMR2Service ( void )
{
    // Clear the hardware interrupt flag.
    IFS0bits.T2IF = 0; 
    
    // Increment the counter (16-bit roll-over counter).
    hwtmr_p1ms_cnt++;
}

uint16_t HwTMRp1msGet ( void )
{
    return hwtmr_p1ms_cnt;
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initialize oscillator configuration.
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////
static void HwOSCInit( void )
{
    // Initialize the oscillator to operate the CPU clock at 40MHz
    // (i.e. 20 MIPS).
    // 
    // M  := PLLDIV + 2
    // N1 := PLLPRE + 2
    // N2 := 2 * ( PLLPOST + 1)
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
    // clock will be set to the same value it already is (not PLL lock time
    // should be needed).
    //
    CLKDIVbits.PLLPRE  =  0;         // PLL input divided by 2.      (N1)
    CLKDIVbits.PLLPOST =  1;         // PLL output divided by 4.     (N2)
    PLLFBDbits.PLLDIV  = 30;         // PLL feedback divisor is 32.  (M)
    
    // Wait for the PLL to lock.
    //
    // Note: The PLL lock time (Tlock) is a maximum of 3.1ms (see datasheet).
    //
    while( OSCCONbits.LOCK == 0 );   
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initialize I2C 1 configuration.
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////
static void HwI2C1Init( void )
{
    // Initialize the I2C hardware for 100KHz operation.  The hardware contains
    // a single I2C module (i.e. I2C1).
    //
    // Fscl =    Fcy  / ( ( I2CxBRG + 2 ) * 2 )
    //      =  20MHz  / ( (      98 + 2 ) * 2 )
    //      = 100KHz
    //
    // Note: when the I2C1 module is enabled the state and direction pins
    // SCL1 & SDA1 are overwritten; therefore, no pin I/O configuration
    // (e.g. register ODCx) is needed.
    //
    // Note: Disabling of the I2C is performed before modifying register.  Out
    // of reset the I2C should be disabled, so disabled is performed purely
    // for robustness.
    //
    I2C1CON1bits.I2CEN = 0;     // Disable I2C1
   
    I2C1BRG = 98;               // Set 100 kHz clock.

    I2C1CON1bits.I2CEN = 1;     // Enable I2C1.
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initialize Timer 1 hardware configuration.
/// @param  None.
/// @return None.
////////////////////////////////////////////////////////////////////////////////
static void HwTMR1Init( void )
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
/// @brief  Initialize Timer 1 hardware configuration.
/// @param  None.
/// @return None.
////////////////////////////////////////////////////////////////////////////////
static void HwTMR2Init( void )
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
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initialize discrete I/O configuration.
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////
static void HwIOInit( void )
{
    
}