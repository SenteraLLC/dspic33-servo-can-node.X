
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
#include "can.h"
#include "cfg.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************
static uint16_t __align( 16 * 16 ) can_msg_buf[ 16 ][ 8 ];

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************
static void CANTxBuildHeader ( CAN_TX_MSG_TYPE_E tx_msg_type, uint16_t msg_buf[ 8 ] );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
void CANInit ( void )
{
    uint8_t node_id;
    
    // Get the node ID - used for filtering received messages for those which
    // are only applicable to the node.
    node_id = CfgNodeIdGet();
    
    RPINR26bits.C1RXR = 0b0010100;  // Assign C1RX peripheral pin to RP20.
    RPOR1bits.RP36R   = 0b001110;   // Assign C1TX peripheral pint to RP36.
    
    C1CTRL1bits.WIN = 0; // Select the control and status registers for visibility in SFRs.
    
    // Request Configuration Mode for the ECAN module.
    //
    // Note: Configuration Mode is the state out of reset.  Configuration Mode
    // is requested for robustness.
    //
    C1CTRL1bits.REQOP = 4;
    
    // Wait for the ECAN module to enter into Configuration Mode
    while(C1CTRL1bits.OPMODE != 4);

    C1CTRL1bits.CSIDL   = 0;        // N/A, b/c CPU idle mode is never entered.
    C1CTRL1bits.CANCKS  = 0;        // Select Fcan = Fp. 
    C1CTRL1bits.CANCAP  = 0;        // CAN message timestamping not used.
    
    C1CTRL2bits.DNCNT   = 0;        // Disable DeviceNet feature since CAN Specification 2.0A protocol is not used.
    
    C1FCTRLbits.DMABS   = 0b100;    // 16 buffers in RAM.
    C1FCTRLbits.FSA     = 0;        // N/A, FIFO not used.
    
    C1INTEbits.IVRIE    = 0;        // Invalid Message Interrupt is disabled.
    C1INTEbits.WAKIE    = 0;        // Bus Wake-up Activity Interrupt is disabled.
    C1INTEbits.ERRIE    = 0;        // Error Interrupt is disabled.
    C1INTEbits.FIFOIE   = 0;        // FIFO Almost Full Interrupt is disabled.
    C1INTEbits.RBOVIE   = 0;        // RX Buffer Overflow Interrupt is disabled.
    C1INTEbits.RBIE     = 0;        // RX Buffer Interrupt is disabled.
    C1INTEbits.TBIE     = 0;        // TX Buffer Interrupt is disabled.
    
    // Fp    = 20MHz
    // Fbaud = 1Mbps
    //
    // Ftq := Time Quantum Frequency (selected as 10MHz - i.e. 10 TQ per bit)
    //
    // BPR = ( Fp    / ( 2 * Ftq   ) ) - 1
    //     = ( 20MHz / ( 2 * 10MHz ) ) - 1
    //     = 0
    //
    // Synch Segment     = 1 TQ (constant)
    // Propagation Delay = 3 TQ
    // Phase Segment 1   = 3 TQ
    // Phase Segment 2   = 3 TQ
    // SJW               = Min( 4, Prop, PS1, PS2 ) = 3 TQ
    //
    C1CFG1bits.SJW      = 2;    // Select SJW (2 = 3 TQ).
    C1CFG1bits.BRP      = 0;    // Select baud rate for expected Ftq.
    
    C1CFG2bits.WAKFIL   = 0;    // N/A, b/c sleep mode not used.
    C1CFG2bits.SEG2PH   = 2;    // Select phase segment 2 time (2 = 3 TQ).
    C1CFG2bits.SEG2PHTS = 1;    // Phase segment 2 set to be programmable.
    C1CFG2bits.SAM      = 1;    // Select three samplings at sample point.
    C1CFG2bits.SEG1PH   = 2;    // Select phase segment 1 time (2 = 3 TQ).
    C1CFG2bits.PRSEG    = 2;    // Select propagation time (2 = 3 TQ).
    
    // Setup pointers for transmit buffers and priority levels.
    //
    C1TR01CONbits.TXEN0     = 1;    // Buffer TRB0 is a transmit buffer.
    C1TR01CONbits.TX0PRI    = 0b11; // Buffer TRB0 is highest priority.
    
    C1TR01CONbits.TXEN1     = 1;    // Buffer TRB1 is a transmit buffer.
    C1TR01CONbits.TX1PRI    = 0b11; // Buffer TRB1 is highest priority.
    
    C1TR23CONbits.TXEN2     = 1;    // Buffer TRB2 is a transmit buffer.
    C1TR23CONbits.TX2PRI    = 0b01; // Buffer TRB2 is low intermediate priority.
    
    C1TR23CONbits.TXEN3     = 1;    // Buffer TRB3 is a transmit buffer.
    C1TR23CONbits.TX3PRI    = 0b01; // Buffer TRB3 is low intermediate priority.
    
    C1TR45CONbits.TXEN4     = 1;    // Buffer TRB4 is a transmit buffer.
    C1TR45CONbits.TX4PRI    = 0b00; // Buffer TRB4 is lowest priority.  
    
    C1TR45CONbits.TXEN5     = 1;    // Buffer TRB5 is a transmit buffer.
    C1TR45CONbits.TX5PRI    = 0b00; // Buffer TRB5 is lowest priority. 
    
    // -------------------------------------------------------------------------
    //
    // The S-Node needs to receive messages with CAN extended identifiers.
    // Three messages are received over the CAN bus.  The following filters
    // are configured to received CAN messages:
    //      Filter 0 - Servo Command         (10)
    //      Filter 1 - Servo Command         (10)
    //      Filter 2 - Servo Command         (10)
    //      Filter 3 - Configuration Write  (800)
    //      Filter 4 - Configuration Read   (801)
    //
    // The Configuration Write and Read commands operate at a low-enough of a
    // rate (and are not flight-critical) so a single receive buffer is used
    // for processing of the messages.
    //
    // The Servo Command message is received at a rate which matches to
    // software cycle rate.  Three message buffers are used to handle
    // non-synchronization of nodes and to address jitter.  The hardware
    // automatically stored a received message at the higher-indexed filter
    // buffer pointer (BUFPNT) when the lower-indexed buffer is full.
    //
    // -------------------------------------------------------------------------
    //
    // Filter 0,1,2:
    //  bits 28-19: Data Type           = 10
    //  bits 18-17: Transfer Type       = 0b11  (Message Unicast)
    //  bits 16-10: Source Node ID      = 0      (FMU)
    //  bits  9- 7: Reserved            = x
    //  bits  6- 0: Destination Node Id = n
    //
    //  bits 28-0 = 0_0000_0101_01 10_0000_00xx_xnnn_nnnn
    //             |      SID    | |         EID        |
    //
    // Filter 3:
    //  bits 28-19: Data Type           = 800
    //  bits 18-17: Transfer Type       = 0b01  (Service Request)
    //  bits 16-10: Source Node ID      = 0     (FMU)
    //  bits  9- 7: Reserved            = x
    //  bits  6- 0: Destination Node Id = n
    //
    //  bits 28-0 = 1_1001_0000_00 10_0000_00xx_xnnn_nnnn
    //             |      SID    | |         EID        |
    //
    // Filter 4:
    //  bits 28-19: Data Type           = 801
    //  bits 18-17: Transfer Type       = 0b01  (Service Request)
    //  bits 16-10: Source Node ID      = 0     (FMU)
    //  bits  9- 7: Reserved            = x
    //  bits  6- 0: Destination Node Id = n
    //  
    //  bits 28-0 = 1_1001_0000_10 10_0000_00xx_xnnn_nnnn
    //             |      SID    | |         EID        |
    //
    C1CTRL1bits.WIN     = 1;    // Select the filters for visibility in SFRs.
    
    C1FEN1              = 0;    // Disabled all filters to start.
    C1FEN1bits.FLTEN0   = 1;    // Enable filter 0.
    C1FEN1bits.FLTEN1   = 1;    // Enable filter 1.
    C1FEN1bits.FLTEN2   = 1;    // Enable filter 2.
    C1FEN1bits.FLTEN3   = 1;    // Enable filter 3.
    C1FEN1bits.FLTEN4   = 1;    // Enable filter 4.

    C1BUFPNT1bits.F0BP  = 8;    // Acceptance Filter 0 to use Message Buffer  8 to store message.
    C1BUFPNT1bits.F1BP  = 9;    // Acceptance Filter 1 to use Message Buffer  9 to store message.
    C1BUFPNT1bits.F2BP  = 10;   // Acceptance Filter 2 to use Message Buffer 10 to store message.
    C1BUFPNT1bits.F3BP  = 11;   // Acceptance Filter 3 to use Message Buffer 11 to store message.   
    C1BUFPNT2bits.F4BP  = 12;   // Acceptance Filter 4 to use Message Buffer 12 to store message.
     
    C1RXF0SIDbits.SID   = 0x015;    // Set filter 0 match values.
    C1RXF0SIDbits.EXIDE = 1;        // Match messages only with extended ID.
    C1RXF0SIDbits.EID   = 0x2;      // Set filter 0 match values.
    C1RXF0EID           = node_id;  // Set filter 0 match values.

    C1RXF1SIDbits.SID   = 0x015;    // Set filter 1 match values.
    C1RXF1SIDbits.EXIDE = 1;        // Match messages only with extended ID.
    C1RXF1SIDbits.EID   = 0x2;      // Set filter 1 match values.
    C1RXF1EID           = node_id;  // Set filter 1 match values.
    
    C1RXF2SIDbits.SID   = 0x015;    // Set filter 2 match values.
    C1RXF2SIDbits.EXIDE = 1;        // Match messages only with extended ID.
    C1RXF2SIDbits.EID   = 0x2;      // Set filter 2 match values.
    C1RXF2EID           = node_id;  // Set filter 2 match values.
    
    C1RXF3SIDbits.SID   = 0x640;    // Set filter 3 match values.
    C1RXF3SIDbits.EXIDE = 1;        // Match messages only with extended ID.
    C1RXF3SIDbits.EID   = 0x2;      // Set filter 3 match values.
    C1RXF3EID           = node_id;  // Set filter 3 match values.
    
    C1RXF4SIDbits.SID   = 0x641;    // Set filter 4 match values.
    C1RXF4SIDbits.EXIDE = 1;        // Match messages only with extended ID.
    C1RXF4SIDbits.EID   = 0x2;      // Set filter 4 match values.
    C1RXF4EID           = node_id;  // Set filter 4 match values.
    
    C1FMSKSEL1bits.F0MSK = 0b00;    // Set filter 0 for mask 0 match.
    C1FMSKSEL1bits.F1MSK = 0b00;    // Set filter 1 for mask 0 match.
    C1FMSKSEL1bits.F2MSK = 0b00;    // Set filter 2 for mask 0 match.
    C1FMSKSEL1bits.F3MSK = 0b00;    // Set filter 3 for mask 0 match.
    C1FMSKSEL1bits.F4MSK = 0b00;    // Set filter 4 for mask 0 match.
    
    C1RXM0SIDbits.SID   = 0x7FF;  // Set mask 0 - match bits 28-18.
    C1RXM0SIDbits.MIDE  = 1;      // Only match extended IDs.
    C1RXM0SIDbits.EID   = 0x3;    // Set mask 0 - match bits 17-16.
    C1RXM0EID           = 0xFC7F; // Set mask 0 - match bits 15-10 & 6-0, ignore bits 9-7.
    
    // Configure DMA0 for CAN1 transmit operation.
    DMA0CONbits.SIZE    = 0;                                                    // Perform word transfers.
    DMA0CONbits.DIR     = 1;                                                    // Transfer from RAM to the peripheral address.
    DMA0CONbits.HALF    = 0;                                                    // Do not generate interrupt when half of data moved.
    DMA0CONbits.NULLW   = 0;                                                    // Normal operation.
    DMA0CONbits.AMODE   = 0b10;                                                 // Peripheral indirect addressing mode.
    DMA0CONbits.MODE    = 0b00;                                                 // Continuous Ping-Pong modes disabled.
    DMA0REQbits.IRQSEL  = 70;                                                   // Associate the DMA channel to IRQ 70 (i.e. CAN1 TX Data Request)
    DMA0CNTbits.CNT     = 7;                                                    // Perform 8 transfers.
    DMA0PAD             = (volatile unsigned int) &C1TXD;                       // Peripheral address of CAN1 transmit data register.
    DMA0STAL            = (unsigned int) &can_msg_buf;                          // Set the DMA0 start address register.    
    DMA0STAH            = 0x0000;                                               // N/A near memory accessed
    DMA0CONbits.CHEN    = 1;                                                    // Enable the DMA0 channel.
    
    // Configure DMA1 for CAN1 receive operation.
    DMA1CONbits.SIZE    = 0;                                                    // Perform word transfers.
    DMA1CONbits.DIR     = 0;                                                    // Transfer from peripheral address to RAM.
    DMA0CONbits.HALF    = 0;                                                    // Do not generate interrupt when half of data moved.
    DMA0CONbits.NULLW   = 0;                                                    // Normal operation.
    DMA1CONbits.AMODE   = 0b10;                                                 // Peripheral indirect addressing mode.
    DMA1CONbits.MODE    = 0b00;                                                 // Continuous Ping-Pong modes disabled.
    DMA0REQbits.IRQSEL  = 34;                                                   // Associate the DMA channel to IRQ 34 (i.e. CAN1 Receive Data Ready)
    DMA0CNTbits.CNT     = 7;                                                    // Perform 8 transfers.
    DMA1PAD             = (volatile unsigned int) &C1RXD;                       // Peripheral address of CAN1 transmit data register.
    DMA1STAL            = (unsigned int) &can_msg_buf;                          // Set the DMA0 start address register.    
    DMA1STAH            = 0x0000;                                               // N/A near memory accessed
    DMA1CONbits.CHEN    = 1;                                                    // Enable the DMA1 channel.            
    
    C1CTRL1bits.WIN = 0; // select the buffer window for visibility in SFRs.
    
    // Request Normal Operating Mode
    C1CTRL1bits.REQOP = 0;
    
    // Wait for the ECAN module to enter into Normal Operating Mode
    while(C1CTRL1bits.OPMODE != 0);
}

void CANTxSet ( CAN_TX_MSG_TYPE_E tx_msg_type, const uint16_t payload[ 4 ] )
{
    // Structure definition of HW elements corresponding the a message type.
    typedef struct
    {
        uint8_t buffer_index;
        volatile uint16_t* trcon_p;
        uint16_t txreq_mask;
       
    } TX_HW_MAP_S;
    
    // Mapping of message types to hardware elements for transmitting the message.
    static const TX_HW_MAP_S tx_hw_map[ CAN_TX_MSG_NUM_OF ] = 
    {
        { 0, &C1TR01CON, 0x0008 },   // CAN_TX_MSG_SERVO_STATUS
        { 1, &C1TR01CON, 0x0800 },   // CAN_TX_MSG_VSENSE_DATA
        { 2, &C1TR23CON, 0x0008 },   // CAN_TX_MSG_NODE_STATUS
        { 3, &C1TR23CON, 0x0800 },   // CAN_TX_MSG_NODE_VER
        { 4, &C1TR45CON, 0x0008 },   // CAN_TX_MSG_CFG_WRITE_RESP
        { 5, &C1TR45CON, 0x0800 },   // CAN_TX_MSG_CFG_READ_RESP
    };
    
    uint8_t buf_idx;
    uint8_t payload_idx;
    
    // Transmission request is not already set - i.e. a tranmission is not
    // already in progress for the transmit buffer ?
    if( ( *tx_hw_map[ tx_msg_type ].trcon_p & tx_hw_map[ tx_msg_type ].txreq_mask ) != 1 )
    {   
        // Copy buffer index to local variable for processing efficiency
        // and reduced line length.
        buf_idx = tx_hw_map[ tx_msg_type ].buffer_index;
        
        // Copy the payload to the transmit buffer.
        for( payload_idx = 0;
             payload_idx < 4;
             payload_idx++ )
        {
            // Note: First 3 words of hardware buffer are used for CAN ID,
            // DLC, and control bits.
            can_msg_buf[ buf_idx ][ payload_idx + 3 ] = payload[ payload_idx ];
        }

        // Build the CAN message header.
        CANTxBuildHeader( tx_msg_type, &can_msg_buf[ buf_idx ][ 0 ] );
        
        // Request (i.e. set request bit to '1') the transmission.
        *tx_hw_map[ tx_msg_type ].trcon_p |= tx_hw_map[ tx_msg_type ].txreq_mask;
    }
}

bool CANRxGet ( CAN_RX_MSG_TYPE_E rx_msg_type, uint16_t payload[ 4 ] )
{
    // Structure definition of HW elements corresponding the a message type.
    typedef struct
    {
        uint8_t buffer_index;
        volatile uint16_t* rxful_p;
        uint16_t rxful_mask;
       
    } RX_HW_MAP_S;
    
    static const RX_HW_MAP_S hw_map_servo_cmd[] = 
    {
        {  8, &C1RXFUL1, 0x0100 },
        {  9, &C1RXFUL1, 0x0200 },
        { 10, &C1RXFUL1, 0x0400 },
        {  0, NULL,      0x0000 },   // NULL terminated
    };
    
    static const RX_HW_MAP_S hw_map_cfg_write[] = 
    {
        { 11, &C1RXFUL1, 0x0800 },
        {  0, NULL,      0x0000 },   // NULL terminated
    };
    
    static const RX_HW_MAP_S hw_map_cfg_read[] = 
    {
        { 12, &C1RXFUL1, 0x1000 },
        {  0, NULL,      0x0000 },   // NULL terminated
    };
    
    static const RX_HW_MAP_S* const rx_hw_map[ CAN_RX_MSG_NUM_OF ] =
    {
        &hw_map_servo_cmd[ 0 ],     // CAN_RX_MSG_SERVO_CMD
        &hw_map_cfg_write[ 0 ],     // CAN_RX_MSG_CFG_WRITE_REQ
        &hw_map_cfg_read[ 0 ],      // CAN_RX_MSG_CFG_READ_REQ
    };
    
    
    uint8_t payload_idx;
    uint8_t map_idx;
    
    bool    data_rx_flag = false;
    uint8_t buf_idx;
    
    // Determine buffer index which contains the freshest data.
    for( map_idx = 0;
         rx_hw_map[ rx_msg_type ][ map_idx ].rxful_p != NULL;
         map_idx++ )
    {
        // Buffer is full ?
        if( ( *rx_hw_map[ rx_msg_type ][ map_idx ].rxful_p & 
                    rx_hw_map[ rx_msg_type ][ map_idx ].rxful_mask ) == 1 )
        {
            // Identify data as received.
            data_rx_flag  = true;
            
            // Copy buffer index to local variable for processing efficiency
            // and reduced line length.
            buf_idx = rx_hw_map[ rx_msg_type ][ map_idx ].buffer_index;

            // Copy payload into supplied buffer.
            for ( payload_idx = 0;
                  payload_idx < 4;
                  payload_idx++ )
            {
                // Note: First 3 words of hardware buffer are used for CAN ID,
                // DLC, and control bits.
                payload[ payload_idx ] = can_msg_buf[ buf_idx ][ payload_idx + 3 ];
            }
            
            // Clear the receiver buffer flag so the hardware will receive a
            // new message into the buffer.
            *rx_hw_map[ rx_msg_type ][ map_idx ].rxful_p |= ~rx_hw_map[ rx_msg_type ][ map_idx ].rxful_mask;
        }
    }
 
    return data_rx_flag;
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief 
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////
static void CANTxBuildHeader ( CAN_TX_MSG_TYPE_E tx_msg_type, uint16_t msg_buf[ 8 ] )
{
    // Union defining the contents of the CAN ID field.
    typedef union
    {
        struct
        {
            uint32_t dest_id    :  7;   // bits 6 - 0
            uint32_t            :  3;   // bits 9 - 7 (reserved)
            uint32_t src_id     :  7;   // bits 16-10
            uint32_t tsf_type   :  2;   // bits 18-17
            uint32_t data_type  : 10;   // bits 28-19
        };
        
        struct
        {
            uint32_t id3        : 11;   // bits 10- 0
            uint32_t id2        : 12;   // bits 22-11
            uint32_t id1        :  6;   // bits 28-23
        };
        
    } CAN_ID_U;
    
    // Structure defining the contents of the CAN header and footer.
    typedef struct
    {
        uint8_t  data_len;
        CAN_ID_U can_id;
        
    } CAN_DATA_S;
    
    // Structure defining the contents of the CAN message header within the
    // hardware buffer.
    typedef struct
    {
        struct
        {
            // Word 1.
            uint16_t ide    :  1;
            uint16_t srr    :  1;
            uint16_t id1    : 11;
            uint16_t        :  3;

            // Word 2.
            uint16_t id2    : 12;
            uint16_t        :  4;

            // Word 3.
            uint16_t dlc    :  4;
            uint16_t rb0    :  1;
            uint16_t        :  3;
            uint16_t rb1    :  1;
            uint16_t rtr    :  1;
            uint16_t id3    :  6; 
        };
        
        uint16_t data_u16[ 3 ];
        
    } CAN_HW_HEADER_U;
    
    // Defined messages' header content and message length.
    static const CAN_DATA_S tx_can_data[ CAN_TX_MSG_NUM_OF ] =
    {
        // CAN_TX_MSG_SERVO_STATUS
        {
            8,              // data_len
            
            {
                {
                    0,          // dest_id      - N/A, broadcast message.
                    0,          // src_id       - N/A, set real-time.        
                    0b10,       // tsf_type     - Message broadcast.
                    20,         // data_type    - 20 identifies Servo Status Message.
                },
            },
        },
        
        // CAN_TX_MSG_VSENSE_DATA
        {
            8,              // data_len
            
            {
                {
                    0,          // dest_id      - N/A, broadcast message.
                    0,          // src_id       - N/A, set real-time.        
                    0b10,       // tsf_type     - Message broadcast.
                    21,         // data_type    - 20 identifies VSENSE Status Message.
                },
            },
        },
        
        // CAN_TX_MSG_NODE_STATUS
        {
            4,              // data_len
            
            {
                {
                    0,          // dest_id      - N/A, broadcast message.
                    0,          // src_id       - N/A, set real-time.        
                    0b10,       // tsf_type     - Message broadcast.
                    770,        // data_type    - 770 identifies Node Status Message.
                },
            },
        },
        
        // CAN_TX_MSG_NODE_VER
        {
            8,              // data_len
            
            {
                {
                    0,          // dest_id      - N/A, broadcast message.
                    0,          // src_id       - N/A, set real-time.        
                    0b10,       // tsf_type     - Message broadcast.
                    771,        // data_type    - 771 identifies Node Type and Version Message.
                },
            },
        },
        
        // CAN_TX_MSG_CFG_WRITE_RESP
        {
            6,              // data_len
            
            {
                {
                    0,          // dest_id      - Send to FMU (ID = 0).
                    0,          // src_id       - N/A, set real-time.        
                    0b00,       // tsf_type     - Service Response.
                    800,        // data_type    - 800 identifies Configuration Write Response Message.
                },
            },
        },
        
        // CAN_TX_MSG_CFG_READ_RESP
        {
            0,              // data_len     - Variable, special case treated with execution.
            
            {
                {
                    0,          // dest_id      - Send to FMU (ID = 0).
                    0,          // src_id       - N/A, set real-time.        
                    0b00,       // tsf_type     - Service Response.
                    801,        // data_type    - 801 identifies Configuration Read Response Message.
                },
            },
        },
    };
    
    
    //
    // START OF OPERATIONAL CODE -----------------------------------------------
    //
    
    CAN_ID_U can_id;
    
    // Define the header content.  Set fields which are identical independent
    // of the message type.
    CAN_HW_HEADER_U tx_hw_header =
    {
        {
            1,            // ide - always recessive.
            1,            // srr - always recessive.
            0,            // id1 - updated in fxn.
            0,            // id2 - updated in fxn.
            0,            // dlc - updated in fxn.
            0,            // rb0 - always dominant.
            0,            // rb1 - always dominant.
            0,            // rtr - always dominant for data frames.
            0,            // id3 - updated in fxn.
        },
    };
    
    uint8_t node_id;
    
    node_id = CfgNodeIdGet();
    
    // Copy the CAN ID from NVM and update the Node ID.
    can_id        = tx_can_data[ tx_msg_type ].can_id;
    can_id.src_id = node_id;
    
    // Populate the header within hardware with the CAN ID.
    tx_hw_header.id1 = can_id.id1;
    tx_hw_header.id2 = can_id.id2;
    tx_hw_header.id3 = can_id.id3;
    
    // Handle the special case of Configuration Read Response which has a
    // variable length.
    if( tx_msg_type == CAN_TX_MSG_CFG_READ_RESP )
    {
        // For the Configuration Read Response message, the first word within
        // the payload (i.e. buffer word 3) identifies the type of data
        // returned.  Possible data includes:
        //
        //  Payload word 3      Description             Length (in bytes)
        //  0                   Node ID                 1
        //  1-6                 PWM coefficients        4 (each)
        //  7-12                VSENSE1 Coefficients    4 (each)
        //  13-18               VSENSE2 Coefficients    4 (each)
        //
        // The data length (dlc) for each Read Response Message is 2 bytes for
        // the type identifier (i.e. buffer word 3) plus the value's length.
        //
        if( msg_buf[ 3 ] == 0 )
        {
            tx_hw_header.dlc = 2 + 1;
        }
        else
        {
            tx_hw_header.dlc = 2 + 4;
        }
    }
    else
    {
        // The message is not a Configuration Read Response.  The length is
        // computed statically.
        tx_hw_header.dlc = tx_can_data[ tx_msg_type ].data_len;
    }
    
    // Copy the hardware header into the transmit buffer.
    msg_buf[ 0 ] = tx_hw_header.data_u16[ 0 ];
    msg_buf[ 1 ] = tx_hw_header.data_u16[ 1 ];  
    msg_buf[ 2 ] = tx_hw_header.data_u16[ 2 ];
}



//  TEMPORARY VARIABLE DEFINITION - DELETE WITH INTEGRATION TESTING PROCESS.
//
//    // Defined messages' header content.
//    static const CAN_ID_U tx_can_id[ CAN_TX_MSG_NUM_OF ] =
//    {
//        // CAN_TX_MSG_SERVO_STATUS
//        {
//            20,         // data_type_id
//            0b10,       // transfer_type - Message broadcast
//            0,          // dest_node_id - N/A, broadcast message (set to 0)
//            8,          // data_len 
//        },
//        
//        // CAN_TX_MSG_VSENSE_DATA
//        {
//            21,         // data_type_id
//            0b10,       // transfer_type - Message broadcast
//            0,          // dest_node_id - N/A, broadcast message (set to 0)
//            8,          // data_len 
//        },
//        
//        // CAN_TX_MSG_NODE_STATUS
//        {
//            770,        // data_type_id
//            0b10,       // transfer_type - Message broadcast
//            0,          // dest_node_id - N/A, broadcast message (set to 0)
//            4,          // data_len 
//        },
//        
//        // CAN_TX_MSG_NODE_VER
//        {
//            771,        // data_type_id
//            0b10,       // transfer_type - Message broadcast
//            0,          // dest_node_id - N/A, broadcast message (set to 0)
//            8,          // data_len 
//        },
//        
//        // CAN_TX_MSG_CFG_WRITE_RESP
//        {
//            800,        // data_type_id
//            0b00,       // transfer_type - Service Response
//            0,          // dest_node_id - send to FMU (ID = 0)
//            6,          // data_len 
//        },
//        
//        // CAN_TX_MSG_CFG_READ_RESP
//        {
//            801,        // data_type_id
//            0b00,       // transfer_type - Service Response
//            0,          // dest_node_id - send to FMU (ID = 0)
//            0,          // data_len - Variable, depends on value read.
//        },
//    };