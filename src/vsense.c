////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief VSENSE signal management.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "vsense.h"
#include "adc.h"
#include "can.h"
#include "cfg.h"
#include "util.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// Scaling factors for VSENSE correction polynomial fields:
//
// - Coefficient scale   = 1E4
// - Input scale         = 1E0
// - Output scale        = 1E2
//
// Rationale:
//  The polynomial equation is implemented using fixed-point math since the 
//  hardware does not natively support floating-point arithmetic and the 
//  execution time of a floating-point implementation exceeds that available
//  for the calculation resolution required (i.e. double-precision).
//
//  Additionally, base2 fixed-point math is used rather than base10 as 
//  64-bit division is required and the time required for a base10 
//  implementation (i.e. integer division rather than bit-shift) exceeds
//  the processing time available.
//
// -----------------------------------------------------------------------------
//
// VSENSE1_QNUM_RAW:
//  The VSENSE input treated with an input scaling of 2^12 so that its value
//  spans 0-1.  This value is up-scaled for internal calculation based on
//  VSENSE1_QNUM_CALC.
//  
// VSENSE1_QNUM_CALC:
//  The servo position command is up-scaled for resolution on internal 
//  calculation.  This is critical for maintained accuracy through the power
//  terms (e.g. vsense^5) of the polynomial equation.
//
// VSENSE*_DIV:
//  The output of the polynomial equation has a scaling based to the 
//  coefficient scaling (i.e. 1E4).  The output is  down-scaled to remove the 
//  input scaling, and get the result term to the required units 
//  (i.e. 1E4 / 1E2 = 1E2).
//
// -----------------------------------------------------------------------------
//
#define VSENSE1_QNUM_RAW       12U  ///< VSENSE1 input Q-number.
#define VSENSE1_QNUM_CALC      30U  ///< VSENSE1 polynomial calculation Q-number.
#define VSENSE1_DIV           100U  ///< VSENSE1 post-calculation division factor.

#define VSENSE2_QNUM_RAW       12U  ///< VSENSE2 input Q-number.
#define VSENSE2_QNUM_CALC      30U  ///< VSENSE2 polynomial calculation Q-number.
#define VSENSE2_DIV           100U  ///< VSENSE2 post-calculation division factor.

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

void VsenseService( void )
{
    // CAN message transmitted ever software cycle (10ms).
    static const uint16_t can_tx_period  = 1;
    static       uint16_t can_tx_timeout = 0;
    
    CAN_TX_VSENSE_DATA_U vsense_msg;
    
    uint16_t vsense1_raw;
    int32_t  vsense1_in;
    int32_t  vsense1_cor_i32;
    int16_t  vsense1_cor;
    int32_t  vsense1_coeff[ CFG_VSENSE1_COEFF_LEN ];
    
    uint16_t vsense2_raw;
    int32_t  vsense2_in;
    int32_t  vsense2_cor_i32;
    int16_t  vsense2_cor;
    int32_t  vsense2_coeff[ CFG_VSENSE2_COEFF_LEN ];
    
    
    ////////////////////////////////////////////////////////////////////////////
    // VSENSE1 Calibration Correction
    ////////////////////////////////////////////////////////////////////////////
    
    // Get vsense signal raw value.
    vsense1_raw = ADCGet( ADC_VSENSE1 );
    
    // Get vsense polynomial coefficient correction values.
    CfgVsense1CoeffGet( &vsense1_coeff[ 0 ] );
    
    // Scale VSENSE1 by the calculation factor.
    vsense1_in = ((int32_t) vsense1_raw) << ( VSENSE1_QNUM_CALC - VSENSE1_QNUM_RAW );
    
    // Perform correction of VSENSE1 value
    vsense1_cor_i32 = UtilPoly32( vsense1_in,
                                  VSENSE1_QNUM_CALC,
                                  &vsense1_coeff[ 0 ], 
                                  CFG_VSENSE1_COEFF_LEN );

    // Down-scale the VSENSE1 result to the correction factors Q-number.
    vsense1_cor = (int16_t) ( vsense1_cor_i32 / VSENSE1_DIV );
    
    
    ////////////////////////////////////////////////////////////////////////////
    // VSENSE2 Calibration Correction
    ////////////////////////////////////////////////////////////////////////////
    
    // Get vsense signal raw value.
    vsense2_raw = ADCGet( ADC_VSENSE2 );
    
    // Get vsense polynomial coefficient correction values.
    CfgVsense2CoeffGet( &vsense2_coeff[ 0 ] );
    
    // Scale VSENSE12 by the calculation factor.
    vsense2_in = ((int32_t) vsense2_raw) << ( VSENSE2_QNUM_CALC - VSENSE2_QNUM_RAW );
    
    // Perform correction of VSENSE2 value
    vsense2_cor_i32 = UtilPoly32( vsense2_in,
                                  VSENSE2_QNUM_CALC,
                                  &vsense2_coeff[ 0 ], 
                                  CFG_VSENSE2_COEFF_LEN );

    // Down-scale the VSENSE2 result to the correction factors Q-number.
    vsense2_cor = (int16_t) ( vsense2_cor_i32 / VSENSE2_DIV );
    
    
    ////////////////////////////////////////////////////////////////////////////
    // VSENSE Annunciation
    ////////////////////////////////////////////////////////////////////////////
    
    // timeout has elapsed since last CAN message transmission.
    can_tx_timeout++;
    if( can_tx_timeout >= can_tx_period )
    {
        // reset timeout
        can_tx_timeout = 0;
        
        // Construct the vsense CAN message.
        vsense_msg.vsense1_raw = vsense1_raw;
        vsense_msg.vsense1_cor = vsense1_cor;
        vsense_msg.vsense2_raw = vsense2_raw;
        vsense_msg.vsense2_cor = vsense2_cor;

        // Send the CAN message.
        CANTxSet( CAN_TX_MSG_VSENSE_DATA, vsense_msg.data_u16 );
    }
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
