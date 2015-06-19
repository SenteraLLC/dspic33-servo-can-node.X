
////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief    
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
#include "cfg.h"
#include "can.h"
#include "nvm.h"
#include "util.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// The Program Memory page is 2048 bytes (i.e. 512 program double-words) in
// length.  The upper word of the 512 program double-words is not used for
// constant data storage (i.e. __pack_upper_byte compiler option not used).
//
// The structure is defined as the size for one page (i.e. padded 
// with 'reserved' bytes) so that an erase operation will not inadvertently
// erase other program components.
//
typedef union
{
    struct
    {
        uint8_t  node_id;                                   // word  0      (note: padded)
        int32_t  pwm_coeff[ CFG_PWM_COEFF_LEN ];            // word  1-12
        int32_t  vsense1_coeff[ CFG_VSENSE1_COEFF_LEN ];    // word 13-24
        int32_t  vsense2_coeff[ CFG_VSENSE2_COEFF_LEN ];    // word 25-36

        uint16_t reserved[ 475 ];                           // word 37-512
    }dstruct;
    
    uint16_t data_u16[ 512 ];
    
} CFG_DATA_U;

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************

// Align the configuration data memory allocation to a Program Memory page.
static const CFG_DATA_U __align( 1024 ) cfg_data =
{
    {
        0x7F,       // Initialize node_id to maximum 7-bit value.
        { 0x76543210 },                                                             // DEGUG CODE TEMP VALUE, SHOULD INITIALIZE TO ZERO ORDER POLYNOMIAL WITH CONSTANT VALUE OF 1.
        { 0xFEDCBA98 },                                                             // DEGUG CODE TEMP VALUE, SHOULD INITIALIZE TO ZERO ORDER POLYNOMIAL WITH CONSTANT VALUE OF 1.
        { 0x11223344 },                                                             // DEGUG CODE TEMP VALUE, SHOULD INITIALIZE TO ZERO ORDER POLYNOMIAL WITH CONSTANT VALUE OF 1.
        { 0x5566 },                                                                 // DEGUG CODE TEMP VALUE, SHOULD INITIALIZE TO ZEROS.
    }
};

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************
static void CfgWrite ( void );
static void CfgRead ( void );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
void CfgService ( void )
{
    // Service a write request.
    CfgWrite();
    
    // Service a read request.
    CfgRead();
}

uint8_t CfgNodeIdGet ( void )
{
    return cfg_data.dstruct.node_id;
}

void CfgPWMCoeffGet ( int32_t pwm_coeff[ CFG_PWM_COEFF_LEN ] )
{
    uint8_t coeff_idx;
    
    for ( coeff_idx = 0;
          coeff_idx < CFG_PWM_COEFF_LEN;
          coeff_idx++ )
    {
        pwm_coeff[ coeff_idx ] = cfg_data.dstruct.pwm_coeff[ coeff_idx ];
    }
}

void CfgVsense1CoeffGet ( int32_t vsense1_coeff[ CFG_VSENSE1_COEFF_LEN ] )
{
    uint8_t coeff_idx;
    
    for ( coeff_idx = 0;
          coeff_idx < CFG_VSENSE1_COEFF_LEN;
          coeff_idx++ )
    {
        vsense1_coeff[ coeff_idx ] = cfg_data.dstruct.vsense1_coeff[ coeff_idx ];
    }
}

void CfgVsense2CoeffGet ( int32_t vsense2_coeff[ CFG_VSENSE2_COEFF_LEN ] )
{
    uint8_t coeff_idx;
    
    for ( coeff_idx = 0;
          coeff_idx < CFG_VSENSE2_COEFF_LEN;
          coeff_idx++ )
    {
        vsense2_coeff[ coeff_idx ] = cfg_data.dstruct.vsense2_coeff[ coeff_idx ];
    }
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief 
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////
static void CfgWrite ( void )
{
    static CFG_DATA_U cfg_data_cpy;
    
    CAN_TX_WRITE_REQ_U  write_req_payload;
    CAN_TX_WRITE_RESP_U write_resp_payload;
            
    bool node_id_update = false;
    
    bool payload_valid;
    bool fault_status;
    
    payload_valid = CANRxGet ( CAN_RX_MSG_CFG_WRITE_REQ, write_req_payload.data_u16 );
    
    // Write request message received ?
    if( payload_valid == true )
    {
        // Copy the configuration data from NVM to RAM.
        cfg_data_cpy.dstruct = cfg_data.dstruct;
        // cfg_data_cpy = cfg_data;
        
        // Update the selected fields in RAM with the new value.
        switch( write_req_payload.cfg_sel )
        {
            case 0:
                cfg_data_cpy.dstruct.node_id = write_req_payload.cfg_val_u8;
                node_id_update =  true;
                break;
                
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                cfg_data_cpy.dstruct.pwm_coeff[ write_req_payload.cfg_sel - 1 ] = write_req_payload.cfg_val_i32;
                break;
            
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                cfg_data_cpy.dstruct.vsense1_coeff[ write_req_payload.cfg_sel - 7 ] = write_req_payload.cfg_val_i32;
                break;
            
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
                cfg_data_cpy.dstruct.vsense1_coeff[ write_req_payload.cfg_sel - 13 ] = write_req_payload.cfg_val_i32;
                break;
            
            default:
                ;
        }
        
        // Erase the NVM page.
        fault_status = NVMErasePage( __builtin_tblpage(   &cfg_data ), 
                                     __builtin_tbloffset( &cfg_data ) );
        
        // Erase operation was successful ?
        if( fault_status == false )
        {
            // Program the updated RAM copy to the NVM page.
            fault_status = NVMProgramPage( cfg_data_cpy.data_u16, 
                                          __builtin_tblpage(   &cfg_data ), 
                                          __builtin_tbloffset( &cfg_data ) );
        }
        
        // Construct the Write Response message
        write_resp_payload.cfg_sel      = write_resp_payload.cfg_sel;
        write_resp_payload.fault_status = fault_status;
        
        // Send the Write Response message.
        CANTxSet ( CAN_TX_MSG_CFG_WRITE_RESP, write_resp_payload.data_u16 );
        
        // Node ID was updated ?
        if( node_id_update == true )
        {
            // Wait for 5ms so the Write Response CAN message has time
            // to be transmitted.
            UtilDelay( 5 );
            
            // Perform a software reset so that the updated Node ID can
            // be used for CAN message filtering.
            __asm__ volatile ("reset");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief 
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////
static void CfgRead ( void )
{
    CAN_TX_READ_REQ_U   read_req_payload;      
    CAN_TX_READ_RESP_U  read_resp_payload;

    bool payload_valid;
    
    payload_valid = CANRxGet ( CAN_RX_MSG_CFG_READ_REQ, read_req_payload.data_u16 );
    
    // Read request message received ?
    if( payload_valid == true )
    {
        // Copy the configuration selection to the response message.
        read_resp_payload.cfg_sel = read_req_payload.cfg_sel;
        
        // Depending on the configuration selection, populate the required
        // data within the response message.
        switch( read_resp_payload.cfg_sel )
        {
            case 0:
                read_resp_payload.cfg_val_u8 = cfg_data.dstruct.node_id;
                break;
                
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                read_resp_payload.cfg_val_i32 = cfg_data.dstruct.pwm_coeff[ read_resp_payload.cfg_sel - 1 ];
                break;
            
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                read_resp_payload.cfg_val_i32 = cfg_data.dstruct.vsense1_coeff[ read_resp_payload.cfg_sel - 7 ];
                break;
            
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
                read_resp_payload.cfg_val_i32 = cfg_data.dstruct.vsense2_coeff[ read_resp_payload.cfg_sel - 13 ];
                break;
            
            default:
                ;
        }
        
        // Send the Read Response message.
        CANTxSet ( CAN_TX_MSG_CFG_READ_RESP, read_resp_payload.data_u16 );
    }
}
