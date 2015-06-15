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
#include "ina219.h"
#include "adc.h"
#include "wdt.h"
#include "rst.h"
#include "pwm.h"
#include "can.h"
#include "cfg.h"
#include "nvm.h"
#include "ver.h"
#include "vsense.h"
#include "servo.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

int main ( void )
{
    // Enable the watchdog timer operation.
    WDTEnable();
    
    // Initialize CPU hardware.
    HwInit();
    ADCInit();
    PWMInit();
    CANInit();
    NVMInit();
    
    // Initialize peripheral hardware.
    INA219Init();
    
    // Determine the processor reset source.
    RSTStartup();
    
    // Enable the hardware timer(s) to start interrupt threads of main
    // processing control-flow.
    HwTMREnable();
    PWMEnable();
    
    // Enable the Global Interrupt flag for executive control-flow.
    INTCON2bits.GIE = 1;   
    
    // Execute background thread infinite-loop.
    while( 1 );
    
    return 0;
} 

void __interrupt( no_auto_psv ) _T1Interrupt ( void )
{   
    // QUESTION: NEED TO CLEAR THE TIMER1 INTERRUPT HARDWARE FLAG
    // DURING THE INTERRUPT ?
    
    ADCService();
    INA219Service();
    
    WDTService();
    VsenseService();
    ServoService();
    CfgService();
    RSTService();
    VerService();
    
}

void __interrupt( no_auto_psv ) _DefaultInterrupt ( void )
{
    // Wait in a infinite loop for a WDT reset.
    while( 1 );
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
