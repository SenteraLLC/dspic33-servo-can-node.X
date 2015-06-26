////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief  V  
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
// - VSENSE calculation scale            = 1E6
// - VSENSE calculation input multiplier = 1E3
// - VSENSE calculation output divisor   = 1E13
//
// Rationale:
//  The polynomial equation is implemented using fixed-point math since the 
//  hardware does not natively support floating-point arithmetic and the 
//  execution time of a floating-point implementation exceeds that available
//  for the calculation resolution required (i.e. double-precision).
//
// -----------------------------------------------------------------------------
//
// VSENSE calculation scale:
//  The VSENSE input is used assuming a radix point at 1.0E6.  This is
//  critical for computation of the higher order power terms (e.g. vsense^5)
//  so that integer saturation is prevented.
//  
// VSENSE calculation input multiplier:
//  The VSENSE input is up-scaled for resolution on internal 
//  calculation.  This is critical for maintained accuracy through the power
//  terms (e.g. vsense^5) of the polynomial equation.
//
// VSENSE calculation output divisor:
//  The output is down-scaled to remove the input scaling, and get the result 
//  term to the required range (i.e. 16-bit signed integer).
//
// -----------------------------------------------------------------------------
//
// Dynamic Range:
//  With a VSENSE input value of [0:4095], the calculation scaled value spans
//  [0:4095000].  With a radix point at 1.0E6, taking this value to the 5th 
//  degree yields a result in the range [0:1,151,514,817], which is within a
//  32-bit integer storage size.
//
//  With coefficient values having a storage size of int32_t
//  (i.e. [-2.15E9:2.15E9]) the maximum un-scaled polynomial result achievable
//  (with a 5th degree polynomial) is approximately [-3.27E18:3.27E18], which
//  is within a int64_t storage size.
//
#define VSENSE1_QNUM_RAW       12U
#define VSENSE1_QNUM_CALC      30U
// #define VSENSE1_QNUM_COR       15U
#define VSENSE1_DIV       100U

#define VSENSE2_QNUM_RAW       12U
#define VSENSE2_QNUM_CALC      30U
// #define VSENSE2_QNUM_COR       15U
#define VSENSE2_DIV       100U

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
    
    // Get vsense signal raw values.
    vsense1_raw = ADCGet( ADC_AIN2 );
    vsense2_raw = ADCGet( ADC_AIN3 );
    
    // Get vsense polynomial coefficient correction values.
    CfgVsense1CoeffGet( &vsense1_coeff[ 0 ] );
    CfgVsense2CoeffGet( &vsense2_coeff[ 0 ] );
    
    // Scale VSENSE1 by the calculation factor.
    vsense1_in = ((int32_t) vsense1_raw) << ( VSENSE1_QNUM_CALC - VSENSE1_QNUM_RAW );
    
    // Perform correction of VSENSE1 value
    vsense1_cor_i32 = UtilPoly32( vsense1_in,
                                  VSENSE1_QNUM_CALC,
                                  &vsense1_coeff[ 0 ], 
                                  CFG_VSENSE1_COEFF_LEN );

    // Down-scale the VSENSE1 result to the correction factors Q-number.
    // vsense1_cor = (int16_t) ( vsense1_cor_i32 >> ( VSENSE1_QNUM_CALC - VSENSE1_QNUM_COR ) );
    vsense1_cor = (int16_t) ( vsense1_cor_i32 / VSENSE1_DIV );
    
    // Scale VSENSE12 by the calculation factor.
    vsense2_in = ((int32_t) vsense2_raw) << ( VSENSE2_QNUM_CALC - VSENSE2_QNUM_RAW );
    
    // Perform correction of VSENSE2 value
    vsense2_cor_i32 = UtilPoly32( vsense2_in,
                                  VSENSE2_QNUM_CALC,
                                  &vsense2_coeff[ 0 ], 
                                  CFG_VSENSE2_COEFF_LEN );

    // Down-scale the VSENSE2 result to the correction factors Q-number.
    // vsense2_cor = (int16_t) ( vsense2_cor_i32 >> ( VSENSE2_QNUM_CALC - VSENSE2_QNUM_COR ) );
    vsense2_cor = (int16_t) ( vsense2_cor_i32 / VSENSE2_DIV );
    
    
    // Construct the vsense CAN message.
    vsense_msg.vsense1_raw = vsense1_raw;
    vsense_msg.vsense1_cor = vsense1_cor;
    vsense_msg.vsense2_raw = vsense2_raw;
    vsense_msg.vsense2_cor = vsense2_cor;
    
    // Send the CAN message.
    CANTxSet ( CAN_TX_MSG_VSENSE_DATA, vsense_msg.data_u16 );
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
