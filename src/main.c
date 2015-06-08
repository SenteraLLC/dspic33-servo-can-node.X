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

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

int main( void )
{
    // Initialize CPU hardware
    HwInit();
    ADCInit();
    
    // Initialize peripheral hardware
    INA219Init();
    
    // Determine the processor reset source.
    RSTStartup();
    
    // Enable the hardware timer(s) to start interrupt threads of main
    // processing control-flow
    HwTMREnable();
    
    // Enable the Global Interrupt flag for executive control-flow
    INTCON2bits.GIE = 1;
    
    // Execute background thread infinite-loop
    while( 1 );
    
    return 0;
} 

void __interrupt( no_auto_psv ) _T1Interrupt( void )
{
    // INPUT
    ADCService();
    INA219Service();
//    CANRxService();
//    CANInService();
    
    // PROCESSING
    WDTService();
//    CalCorService();
    
    // OUTPUT
//    PWMOutService();
//    CANTxService();
//    CANOutService();
}

void __interrupt( no_auto_psv ) _DefaultInterrupt( void )
{
    // Wait in a infinite loop for a WDT reset.
    while( 1 );
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
