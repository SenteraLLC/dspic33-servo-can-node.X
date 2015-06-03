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
    
    // Initialize peripheral hardware
    INA219Init();
    
    // Enable the hardware timer(s) to start interrupt threads of main
    // processing control-flow
    HwTMREnable();
    
    // Enable interrupts for executive control-flow
    //__builtin_enable_interrupts();
    INTCON2bits.GIE = 1;        // Global Interrupt Enable     EQUIVALENT TO __builtin_enable_interrupts ??
    
    // Execute background thread infinite-loop
    while( 1 );
    
    return 0;
} 

void __interrupt( no_auto_psv ) _T1Interrupt( void )
{
    // INPUT
//    ATDService();
    INA219Service();
//    CANRxService();
//    CANInService();
    
    // PROCESSING
    HwWDTService();
//    CalCorService();
    
    // OUTPUT
//    PWMOutService();
//    CANTxService();
//    CANOutService();
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
