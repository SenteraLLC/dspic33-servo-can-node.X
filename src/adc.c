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
#include "adc.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************
static uint16_t adc_val[ ADC_AIN_MAX ];

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

// Note: The ADC hardware takes at most 20us (tDPU) to stabilize once the 
// module is enabled (i.e. bit ADON = 1).  The ADC result during this time
// is indeterminate and therefore should not be used.
void ADCInit ( void )
{
// Note: The "FORM" fields allows selection of signed integer representation.
// Would this be better for the signed position (radians) outputs?  Or will
// this just roll into the calibration correction equation?  What's been done
// before?

// *** NOTE: ***
// In order to use port pins for I/O functionality with digital modules, such as timers,
// UARTs, etc., the corresponding ANSELx bits must be cleared.  I/O pins that are shared with any analog input pin
// (i.e., ANx) are always analog pins by default after any Reset.

    
    
    AD1CON1bits.ADON    = 0;        // Turn ADC1 off - required for updating several ADC registers. Should already be 0 from reset value, but included for robustness.
    AD1CON1bits.ADSIDL  = 0;        // N/A, set to Hw default.  Idle mode not entered.
    AD1CON1bits.ADDMABM = 0;        // N/A, set to Hw default.  DMA not used.
    AD1CON1bits.AD12B   = 1;        // 12-bit, 1-channel (CH0) ADC operation.
    AD1CON1bits.FORM    = 0b00;     // Unsigned integer output format (right-aligned)
    AD1CON1bits.SSRC    = 0b111;    // Automatically start conversion when sampling is done.
    AD1CON1bits.SSRCG   = 0;
    AD1CON1bits.SIMSAM  = 0;        // Sample CH0 inputs in sequence.
    // AD1CON1bits.ASAM    = 1;        // Automatic sampling (i.e. not manually controlled).
    AD1CON1bits.ASAM    = 0;        // Manual sampling - sampling begins when SAMP bit is set.

// NOTE: INTERRUPT MUST BE USED IN CHANNEL SCAN MODE ? CAN'T PERFORM CONTINUOUS CONVERSIONS ? 
    AD1CON2bits.VCFG    = 0b000;    // N/A, set to Hw default. AVdd always used for high reference, AVss always used for low reference.
    // AD1CON2bits.CSCNA   = 1;        // Select input channel scan mode.
    AD1CON2bits.CSCNA   = 0;        // Inputs are not scanned.
    AD1CON2bits.CHPS    = 0;        // N/A, set to Hw default.  Single channel (CH0) only possible in 12-bit mode.
    AD1CON2bits.BUFS    = 0;        // N/A, set to Hw default.  Buffer fill status not valid since BUFM = 1.
    AD1CON2bits.SMPI    = 0b0000;   // Do not generate interrupts after conversions.
    // AD1CON2bits.SMPI    = 0b0001;   // Sent to number of inputs scanned minus 1.
    AD1CON2bits.BUFM    = 0;        // Always start filling the buffer from the start address.
    AD1CON2bits.ALTS    = 0;        // Do not use Alternate Input Selection mode.
    
    // Note: Per the DSCs electrical characteristic, the maximum Fad supported
    // for 12-bit conversions is: ~8.5 MHz.
    //
    // Fad  =    Fcy / ( ADCS + 1 )
    //      =  20MHz / (   99 + 1 )
    //      = 200KHz
    //
    // Fsmp =    Fad / SAMC
    //      = 200KHz /   20
    //      =  10KHz
    //
    // (for 12-bit conversion):
    // Fconv = Fad      / 14
    //       = 200KHz   / 14
    //       = 14.29KHz
    //
    AD1CON3bits.ADRC = 0;           // Clock derived from system clock.
    AD1CON3bits.SAMC = 20;          // Set sampling frequency (Fsmp).
    AD1CON3bits.ADCS = 19;          // Set conversion frequency (Fconv).
    
    AD1CON4bits.ADDMAEN = 0;        // Conversion results stored in ADC buffer, DMA not used.
    AD1CON4bits.DMABL   = 0;        // N/A, set to Hw default. DMA not used.
    
    AD1CHS123bits.CH123SB2 = 0;     // N/A, set to Hw default. CH0 is only used channel.
    AD1CHS123bits.CH123SB1 = 0;     // N/A, set to Hw default. CH0 is only used channel.
    AD1CHS123bits.CH123NB1 = 0;     // N/A, set to Hw default. CH0 is only used channel.
    AD1CHS123bits.CH123NB0 = 0;     // N/A, set to Hw default. CH0 is only used channel.
    AD1CHS123bits.CH123SB0 = 0;     // N/A, set to Hw default. CH0 is only used channel.

    AD1CHS0bits.CH0NB = 0;          // N/A, set to Hw default. MUX B not used.
    AD1CHS0bits.CH0SB = 0;          // N/A, set to Hw default. MUX B not used.
    AD1CHS0bits.CH0NA = 0;          // Select Vref- for CH0 negative input.
    
    // AD1CSSLbits.CSS2 = 1;           // Select AN2 (RB0) for input scan.
    // AD1CSSLbits.CSS3 = 1;           // Select AN3 (RB1) for input scan.
    
    
    ANSELBbits.ANSB0 = 1;           // Configure PortB-Pin0 (RB0) for 'analog' operation.
    ANSELBbits.ANSB1 = 1;           // Configure PortB-Pin1 (RB1) for 'analog' operation.
    
    TRISBbits.TRISB0 = 1;           // Configure PortB-Pin0 (RB0) for 'input' operation.
    TRISBbits.TRISB1 = 1;           // Configure PortB-Pin1 (RB1) for 'input' operation.
    

    AD1CON1bits.ADON = 0;           // Turn ADC1 on.
}

void ADCService ( void )
{
// NOTE: UPDATE TO FOR-LOOP IMPLEMENTATION ONCE WORKING.
    
// QUESTION: HOW IS ADC BUFFER MANAGED? AUTOMATICALLY LOOPS OVER AFTER BUFFER IS FILLED (I.E. 16 CONVERSIONS)?
// OR START OVER AT START AFTER THE PERFORMING OF EACH CONVERSION SEQUENCE?
    
    AD1CHS0bits.CH0SA = 2;  // Channel 0 positive input is AN2.

    // Clear the done bit.
    AD1CON1bits.DONE = 0;

    // Start the sampling sequence (automatically following by conversion)
    AD1CON1bits.SAMP = 1;

    // Wait for the conversion to complete.
    while( AD1CON1bits.DONE == 0 );

    // Read the conversion into module data.
    adc_val[ ADC_AIN2 ] = ADC1BUF0;

    AD1CHS0bits.CH0SA = 3;  // Channel 0 positive input is AN3.

    // Clear the done bit.
    AD1CON1bits.DONE = 0;

    // Start the sampling sequence (automatically following by conversion)
    AD1CON1bits.SAMP = 1;

    // Wait for the conversion to complete.
    while( AD1CON1bits.DONE == 0 );

    // Read the conversion into module data.
    adc_val[ ADC_AIN3 ] = ADC1BUF0; 
}

uint16_t ADCGet ( ADC_AIN_E adc_sel )
{
    return ( adc_val[ adc_sel ] );
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////