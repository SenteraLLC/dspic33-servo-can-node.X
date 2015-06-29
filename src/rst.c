
////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief  ???  
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
#include "rst.h"
#include "can.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// The Reset Control Register (RCON) contains flags identifying reset conditions
// in addition to other status registers.  The mask defines values for
// selecting only the applicable reset condition bits from the RCON register.
//
//  bit 15 - TRAPR: Trap Reset Flag bit
//  bit 14 - IOPUWR: Set for the following conditions:
//              * Illegal Opcode Reset
//              * Illegal Address Mode Reset
//              * Uninitialized W Register Access Reset
//              * Security Reset
//  bit  9 - CM: Configuration Mismatch Flag bit
//  bit  7 - EXTR: External Reset (MCLR) Pin bit
//  bit  6 - SWR: Software RESET (Instruction) Flag bit
//  bit  4 - WDTO: Watchdog Timer Time-out Flag bit
//  bit  1 - BOR: Brown-out Reset Flag bit
//  bit  0 - POR: Power-on Reset Flag bit
//
#define RST_MASK            0xC2D3
#define RST_MASK_POR        0x0001
#define RST_MASK_BOR        0x0002
#define RST_MASK_SWR        0x0040
#define RST_MASK_SW_FAULT   0xC290

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************
static uint16_t rst_cond; 
static uint16_t rst_detail; 

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
void RSTStartup ( void )
{
    // Update the reset detail with the applicable bits from the Reset
    // Control Register.
    rst_detail = RCON & RST_MASK;
    
    // Clear the reset condition bits so that the condition can be identified
    // on the next processor reset.
    RCON &= ~RST_MASK;
    
    // Roll-up the reset detail into an easily recognizable identification 
    // of what caused the reset condition.
    //
    // Note: The ordering of the if-else conditions are necessary for expected
    // reset condition identification, since multiple bits within the reset
    // detail can be set (e.g. both POR and BOR are set on a power-on reset
    // condition).  The priority of annunciation is apparent from the ordering
    // of statements within the if-else conditions.
    //
    if( ( rst_detail & RST_MASK_POR ) != 0 )
    {
        rst_cond = 1U;
    }
    else
    if( ( rst_detail & RST_MASK_BOR ) != 0 )
    {
        rst_cond = 2U;
    }
    else
    if( ( rst_detail & RST_MASK_SWR ) != 0 )
    {
        rst_cond = 3U;
    }
    else
    if( ( rst_detail & RST_MASK_SW_FAULT ) != 0 )
    {
        rst_cond = 4U;
    }
    else
    {
        // Note: else-clause included for completeness.  All processor resets
        // should result in a reset condition being identified.
        rst_cond = 0U;
    }
}

void RSTService ( void )
{
    CAN_TX_NODE_STATUS_U node_status_msg;
    
    // Construct the Node Status CAN message.
    node_status_msg.reset_condition = rst_cond;
    node_status_msg.reset_detail    = rst_detail;
    
    // Send the Node Status message.
    CANTxSet ( CAN_TX_MSG_NODE_STATUS, node_status_msg.data_u16 );
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************