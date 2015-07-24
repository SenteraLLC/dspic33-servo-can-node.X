////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Reset condition management.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

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
#define RST_MASK            0xC2D3  ///< Reset conditions (register RCON) mask.
#define RST_MASK_POR        0x0001  ///< Power-on Reset (register RCON) mask.
#define RST_MASK_BOR        0x0002  ///< Brown-out Reset (register RCON) mask.
#define RST_MASK_SWR        0x0040  ///< Software Reset expected (register RCON) mask.
#define RST_MASK_SW_FAULT   0xC290  ///< Software Reset fault (register RCON) mask.

/// List of reset conditions.
typedef enum
{
    RST_COND_UNUSED,    ///< Unused/invalid Reset condition.
    
    RST_COND_POR,       ///< Power-on Reset condition.
    RST_COND_BOR,       ///< Brown-out Reset condition.
    RST_COND_SWR,       ///< Software Reset (expected) condition.
    RST_COND_SW_FAULT   ///< Software Reset (fault) condition.
    
} RST_COND_E;

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

/// Condition which caused the reset.
static RST_COND_E rst_cond;

/// @brief  Detail into reset conditions.
///
/// @note   The value is a copy of hardware register 'RCON' masked with
///         'Reset conditions mask', read during startup.
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
        rst_cond = RST_COND_POR;
    }
    else
    if( ( rst_detail & RST_MASK_BOR ) != 0 )
    {
        rst_cond = RST_COND_BOR;
    }
    else
    if( ( rst_detail & RST_MASK_SWR ) != 0 )
    {
        rst_cond = RST_COND_SWR;
    }
    else
    if( ( rst_detail & RST_MASK_SW_FAULT ) != 0 )
    {
        rst_cond = RST_COND_SW_FAULT;
    }
    else
    {
        // Note: else-clause included for completeness.  All processor resets
        // should result in a reset condition being identified.
        rst_cond = RST_COND_UNUSED;
    }
}

void RSTService ( void )
{
    // CAN message transmitted ever 50 software cycles (10ms * 50 = 500ms).
    static const uint16_t can_tx_period  = 50;
    static       uint16_t can_tx_timeout = 0;
    
    CAN_TX_NODE_STATUS_U node_status_msg;
    
    // timeout has elapsed since last CAN message transmission.
    can_tx_timeout++;
    if( can_tx_timeout >= can_tx_period )
    {
        // reset timeout
        can_tx_timeout = 0;
        
        // Construct the Node Status CAN message.
        node_status_msg.reset_condition = (uint16_t) rst_cond;
        node_status_msg.reset_detail    = rst_detail;

        // Send the Node Status message.
        CANTxSet( CAN_TX_MSG_NODE_STATUS, node_status_msg.data_u16 );
    }
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
