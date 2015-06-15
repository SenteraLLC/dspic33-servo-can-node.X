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
#include "vsense.h"
#include "adc.h"
#include "can.h"
#include "cfg.h"
#include "util.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************
void VsenseService( void )
{
    CAN_TX_VSENSE_DATA_U vsense_msg;
    
    uint16_t vsense1_raw;
    int16_t  vsense1_cor;
    int32_t  vsense1_coeff[ CFG_VSENSE1_COEFF_LEN ];
    
    uint16_t vsense2_raw;
    int16_t  vsense2_cor;
    int32_t  vsense2_coeff[ CFG_VSENSE2_COEFF_LEN ];
    
    // Get vsense signal raw values.
    vsense1_raw = ADCGet( ADC_AIN2 );
    vsense2_raw = ADCGet( ADC_AIN3 );
    
    // Get vsense polynomial coefficient correction values.
    CfgVsense1CoeffGet( &vsense1_coeff[ 0 ] );
    CfgVsense2CoeffGet( &vsense2_coeff[ 0 ] );
    
    // Perform correction of vsense signals.
    vsense1_cor = UtilPolyMul( vsense1_raw, &vsense1_coeff[ 0 ], CFG_VSENSE1_COEFF_LEN );
    vsense2_cor = UtilPolyMul( vsense2_raw, &vsense2_coeff[ 0 ], CFG_VSENSE2_COEFF_LEN );
    
    // Construct the vsense CAN message.
    vsense_msg.vsense1_raw = vsense1_raw;
    vsense_msg.vsense1_cor = vsense1_cor;
    vsense_msg.vsense2_raw = vsense2_raw;
    vsense_msg.vsense2_cor = vsense2_cor;
    
    // Send the CAN message.
    CANTxSet ( CAN_TX_MSG_VSENSE_DATA, vsense_msg.data_u16 );
}

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
