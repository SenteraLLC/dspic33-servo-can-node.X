////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Discrete I/O (DIO) driver.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "dio.h"

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

void DIOInit( void )
{
    // Set unused DSC pins as discrete outputs and drive to a logic-low (see
    // dsPIC33EVXXXGM00X/10X Family datasheet - section 2.8).
    // 
    // Unused processor pins include:
    //  - Pin  7: OSC2/CLKO/RPI19/RA3
    //  - Pin 13: OA5OUT/AN25/C5IN4-/SCK1/RP39/INT0/RB7
    //  - Pin 14: AN26/CVREF1O/CVREF2O/ASCL1/SDO1/RP40/T4CK/RB8
    //  - Pin 15: OA5IN-/AN27/C5IN1-/ASDA1/SDI1/RP41/RB9
    //  - Pin 20: RPI44/PWM1H2/RB12
    //  - Pin 21: RPI45/PWM1L2/CTPLS/RB13
    //  - Pin 22: RPI46/PWM1H1/T3CK/RB14
    //  - Pin 23: RPI47/PWM1L1/T5CK/RB15
    //  - Pin 27: OA2OUT/AN0/C2IN4-/C4IN3-/RPI16/RA0
    //  - Pin 28: OA2IN+/AN1/C2IN1+/RPI17/RA1
    //
    // Note: Pins with AN* designation must have ANSELx selection clear to 
    // enable digital operation.
    //
    // Note: The corresponding bit in register TRISxx is cleared to set the
    // pin as an output, and the corresponding bit in register PORTxx is
    // cleared to set the pin as a logic-low state.
    
    // Pin 7.
    TRISAbits.TRISA3  = 0;
    PORTAbits.RA3     = 0;
    
    // Pin 13.
    ANSELBbits.ANSB7  = 0;
    TRISBbits.TRISB7  = 0;
    PORTBbits.RB7     = 0;
    
    // Pin 14.
    ANSELBbits.ANSB8  = 0;
    TRISBbits.TRISB8  = 0;
    PORTBbits.RB8     = 0;
    
    // Pin 15.
    ANSELBbits.ANSB9  = 0;
    TRISBbits.TRISB9  = 0;
    PORTBbits.RB9     = 0;
    
    // Pin 20.
    TRISBbits.TRISB12 = 0;
    PORTBbits.RB12    = 0;
    
    // Pin 21.
    TRISBbits.TRISB13 = 0;
    PORTBbits.RB13    = 0;
    
    // Pin 22.
    TRISBbits.TRISB14 = 0;
    PORTBbits.RB14    = 0;
    
    // Pin 23.
    TRISBbits.TRISB15 = 0;
    PORTBbits.RB15    = 0;
    
    // Pin 27.
    ANSELAbits.ANSA0  = 0;
    TRISAbits.TRISA0  = 0;
    PORTAbits.RA0     = 0;
    
    // Pin 28.
    ANSELAbits.ANSA1  = 0;
    TRISAbits.TRISA1  = 0;
    PORTAbits.RA1     = 0;
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************