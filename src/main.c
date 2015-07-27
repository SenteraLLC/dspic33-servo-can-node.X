////////////////////////////////////////////////////////////////////////////////
/// @file   
/// @brief  Executive control-flow.
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

#include "adc.h"
#include "can.h"
#include "cfg.h"
#include "dio.h"
#include "i2c.h"
#include "ina219.h"
#include "nvm.h"
#include "osc.h"
#include "pwm.h"
#include "rst.h"
#include "servo.h"
#include "tmr.h"
#include "ver.h"
#include "vsense.h"
#include "wdt.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// DSPIC33EV256GM102 Configuration Bit Settings
// FSEC
#pragma config BWRP     = OFF           // Boot Segment Write-Protect Bit (Boot Segment may be written)
#pragma config BSS      = DISABLED      // Boot Segment Code-Protect Level bits (No Protection (other than BWRP))
#pragma config BSS2     = OFF           // Boot Segment Control Bit (No Boot Segment)
#pragma config GWRP     = OFF           // General Segment Write-Protect Bit (General Segment may be written)
#pragma config GSS      = DISABLED      // General Segment Code-Protect Level bits (No Protection (other than GWRP))
#pragma config CWRP     = OFF           // Configuration Segment Write-Protect Bit (Configuration Segment may be written)
#pragma config CSS      = DISABLED      // Configuration Segment Code-Protect Level bits (No Protection (other than CWRP))
#pragma config AIVTDIS  = DISABLE       // Alternate Interrupt Vector Table Disable Bit  (Disable Alternate Vector Table)

// FBSLIM
#pragma config BSLIM    = 0x1FFD        // Boot Segment Code Flash Page Address Limit Bits (Boot Segment Flash Page Address Limit (0-0x1FFF)) - N/A Alternative interrupts not used.

// FOSCSEL
#pragma config FNOSC    = PRIPLL        // Initial oscillator Source Selection Bits (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config IESO     = OFF           // Two Speed Oscillator Start-Up Bit (Start up device with user selected oscillator source)

// FOSC
#pragma config POSCMD   = EC            // Primary Oscillator Mode Select Bits (EC (External Clock) mode)
#pragma config OSCIOFNC = ON            // OSC2 Pin I/O Function Enable Bit (OSC2 is general purpose digital I/O pin)
#pragma config IOL1WAY  = ON            // Peripheral Pin Select Configuration Bit (Allow Only One reconfiguration)
#pragma config FCKSM    = CSDCMD        // Clock Switching Mode Bits (Both Clock Switching and Fail-safe Clock Monitor are disabled)
#pragma config PLLKEN   = ON            // PLL Lock Enable Bit (Clock switch to PLL source will wait until the PLL lock signal is valid)

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
#pragma config WDTPOST  = PS32          // Select WDT postscaler = 32.
#pragma config WDTPRE   = PR32          // Select SDT prescaler = 32.
#pragma config FWDTEN   = ON_SWDTEN     // WDT disabled by default. WDT enabled in software.
#pragma config WINDIS   = OFF           // Operate WDT in Non-Window Mode.
#pragma config WDTWIN   = WIN25         // N/A - b/c of 'WINDIS' setting.

// FPOR
#pragma config BOREN0   = ON            // Brown Out Reset Detection Bit (BOR is Enabled)

// FICD
#pragma config ICS      = PGD1          // ICD Communication Channel Select Bits (Communicate on PGEC1 and PGED1)

// FDMTINTVL
#pragma config DMTIVTL  = 0xFFFF        // Lower 16 Bits of 32 Bit DMT Window Interval (Lower 16 bits of 32 bit DMT window interval (0-0xFFFF))

// FDMTINTVH
#pragma config DMTIVTH  = 0xFFFF        // Upper 16 Bits of 32 Bit DMT Window Interval (Upper 16 bits of 32 bit DMT window interval (0-0xFFFF))

// FDMTCNTL
#pragma config DMTCNTL  = 0xFFFF        // Lower 16 Bits of 32 Bit DMT Instruction Count Time-Out Value (Lower 16 bits of 32 bit DMT instruction count time-out value (0-0xFFFF))

// FDMTCNTH
#pragma config DMTCNTH  = 0xFFFF        // Upper 16 Bits of 32 Bit DMT Instruction Count Time-Out Value (Upper 16 bits of 32 bit DMT instruction count time-out value (0-0xFFFF))

// FDMT
#pragma config DMTEN    = DISABLE       // Dead Man Timer Enable Bit (Dead Man Timer is Disabled and can be enabled by software)

// FDEVOPT
#pragma config PWMLOCK  = OFF           // PWM Lock Enable Bit (Certain PWM registers may only be written after key sequence)
#pragma config ALTI2C1  = OFF           // Alternate I2C1 Pins Selection Bit (I2C1 mapped to SDA1/SCL1 pins)

// FALTREG
#pragma config CTXT1    = NONE          // Interrupt Priority Level (IPL) Selection Bits For Alternate Working Register Set 1 (Not Assigned)
#pragma config CTXT2    = NONE          // Interrupt Priority Level (IPL) Selection Bits For Alternate Working Register Set 2 (Not Assigned)

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  C-Environment control-flow entry.
///
/// @return Zero is returned if function exits.
///
/// @note   Function implements an infinite loop, therefore function return
///         is not expected.
////////////////////////////////////////////////////////////////////////////////
int main ( void )
{
    // Enable the watchdog timer operation.
    WDTEnable();
    
    // Initialize CPU hardware.
    OSCInit();
    TMRInit();
    DIOInit();
    ADCInit();
    PWMInit();
    CANInit();
    NVMInit();
    I2CInit();
    
    // Initialize peripheral hardware.
    INA219Init();
    
    // Determine the processor reset source.
    RSTStartup();
    
    // Enable the hardware timer(s) to start interrupt thread(s) of main
    // processing control-flow.
    //
    // PWM is enabled at same time as interrupt timer(s) so that PWM cycle
    // is synchronized with processing cycle.  This is not required, but
    // yields a more deterministic design which yields easier identification
    // of the effect of the PWM control on the PWM applied.
    //
    TMR1Enable();
    PWMEnable();
    
    // Enable the Global Interrupt flag for executive control-flow.
    INTCON2bits.GIE = 1;
    
    // Execute background thread infinite-loop.
    while( 1 );
    
    return 0;
} 

////////////////////////////////////////////////////////////////////////////////
/// @brief  Main processing thread (10ms period).
///
/// This interrupt serves as the primary processing thread and is triggered
/// by the Timer1 10ms interrupt.  Interrupt priority is configured as '1', so
/// that the interrupt will preempt background thread execution.
////////////////////////////////////////////////////////////////////////////////
void __interrupt( no_auto_psv ) _T1Interrupt ( void )
{    
    // INPUT - Aquire input signals for software cycle execution.
    ADCService();
    INA219Service();
    
    // PROCESS & OUTPUT - Perform processing and output signals for software
    // cycle execution.
    WDTService();
    VsenseService();
    ServoService();
    CfgService();
    RSTService();
    VerService();
    
    // Service the timer 1 interrupt.
    TMR1Service();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  System timer update (0.1ms period)
///
/// This interrupt serves to update the system time and is triggered by the
/// Timer2 0.1ms interrupt.  Interrupt priority is configured as '2', so that
/// the interrupt will preempt background thread and T1Interrupt thread
/// execution.
////////////////////////////////////////////////////////////////////////////////
void __interrupt( no_auto_psv ) _T2Interrupt ( void )
{
    // Service the timer 2 interrupt.
    TMR2Service();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Unused ISR trap.
///
/// ISR executes an infinite loop so that the WDT will reset the processor.
////////////////////////////////////////////////////////////////////////////////
void __interrupt( no_auto_psv ) _DefaultInterrupt ( void )
{
    while( 1 );
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
