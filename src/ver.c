////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Version management.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "ver.h"
#include "can.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

/// The node type - value of '1' identifies node as a Servo CAN Node.
static const uint8_t  node_type = 1;

static const uint8_t  rev_ver   = 0;    ///< Version revision number.
static const uint8_t  min_ver   = 0;    ///< Version minor number.
static const uint8_t  maj_ver   = 1;    ///< Version major number.

/// The serial number - set during manufacturing.
///
/// @note The serial number is set to the starting address of Program
///       memory.
///
 static const uint32_t __attribute__((space(psv))) __at(0x200) serial_num = 0;

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

void VerService ( void )
{
    // CAN message transmitted ever 50 software cycles (10ms * 50 = 500ms).
    static const uint16_t can_tx_period  = 50;
    static       uint16_t can_tx_timeout = 0;
    
    CAN_TX_NODE_VER_U version_msg;
    
    // timeout has elapsed since last CAN message transmission.
    can_tx_timeout++;
    if( can_tx_timeout >= can_tx_period )
    {
        // reset timeout
        can_tx_timeout = 0;
        
        // Construct the Version CAN message.
        version_msg.node_type  = node_type;
        version_msg.rev_ver    = rev_ver;
        version_msg.min_ver    = min_ver;
        version_msg.maj_ver    = maj_ver;
        version_msg.serial_num = serial_num;

        // Send the Version message.
        CANTxSet( CAN_TX_MSG_NODE_VER, version_msg.data_u16 );
    }
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
