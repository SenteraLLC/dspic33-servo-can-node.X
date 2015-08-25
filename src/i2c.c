////////////////////////////////////////////////////////////////////////////////
/// @file   
/// @brief  Inter-Integrated Circuit (I2C) driver.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "i2c.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

/// I2C communication error latch (i.e. not cleared).
static bool i2c_error_latch = false;

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

static void     I2CStartSeq ( void );
static void     I2CTxSeq    ( uint8_t tx_data );
static uint8_t  I2CRxSeq    ( void );
static void     I2CStopSeq  ( void );

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

void I2CInit( void )
{
    // Initialize the I2C hardware for 100KHz operation.  The hardware contains
    // a single I2C module (i.e. I2C1).
    //
    // Fscl =    Fcy  / ( ( I2CxBRG + 2 ) * 2 )
    //      =  20MHz  / ( (      98 + 2 ) * 2 )
    //      = 100KHz
    //
    // Note: when the I2C1 module is enabled the state and direction pins
    // SCL1 & SDA1 are overwritten; therefore, no pin I/O configuration
    // (e.g. register ODCx) is needed.
    //
    // Note: Disabling of the I2C is performed before modifying register.  Out
    // of reset the I2C should be disabled, so disabled is performed purely
    // for robustness.
    //
    I2C1CON1bits.I2CEN = 0;     // Disable I2C1
   
    I2C1BRG = 98;               // Set 100 kHz clock.

    I2C1CON1bits.I2CEN = 1;     // Enable I2C1.
}

void I2CWrite( uint8_t saddr, const uint8_t* data, uint8_t data_len )
{
    uint8_t data_idx;
    
    // Perform a start sequence.
    I2CStartSeq();
    
    // Transmit the slave address and W/R byte.
    //
    // Note: The slave address occupies bits 7-1 of the transmitted byte.  The
    // write/read command occupies bit 0 of the transmitted byte.  A write
    // operation is identified by setting bit 0 as '0'.
    I2CTxSeq( saddr << 1 );
    
    // Transmit the supplied data.
    for ( data_idx = 0U;
          data_idx < data_len;
          data_idx++ )
    {
        I2CTxSeq( data[ data_idx ] );
    }

    // Perform a stop sequence.
    I2CStopSeq();
}

void I2CRead( uint8_t saddr, uint8_t* data, uint8_t data_len )
{
    int16_t data_idx;
    
    // Perform a start sequence.
    I2CStartSeq();
    
    // Transmit the slave address and W/R byte.
    //
    // Note: The slave address occupies bits 7-1 of the transmitted byte.  The
    // write/read command occupies bit 0 of the transmitted byte.  A read
    // operation is identified by setting bit 0 as '1'.
    //
    I2CTxSeq( ( saddr << 1 ) | 0b1 );
    
    // Stored received data in the supplied buffer.
    //
    // Note: Data transfer via I2C is MSB first and integer values are stored
    // within memory in little-endian format.  Therefore, storing of I2C data
    // needs to begin at the end of the supplied data array.
    //
    for ( data_idx  = data_len - 1;
          data_idx >= 0;
          data_idx-- )
    {
        data[ data_idx ] = I2CRxSeq();
    }

    // Perform a stop sequence.
    I2CStopSeq();
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  Start an I2C sequence.
////////////////////////////////////////////////////////////////////////////////
static void I2CStartSeq ( void )
{
    // Enable a Start sequence.
    I2C1CON1bits.SEN = 1;

    // Wait for Start sequence to complete.
    while( I2C1CON1bits.SEN == 1 );
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Transmit the data byte.
///
/// @param  tx_data
///             Data byte to transmit.
////////////////////////////////////////////////////////////////////////////////
static void I2CTxSeq ( uint8_t tx_data )
{
    // Write the data to the transmit register.
    I2C1TRN = tx_data;

    // Wait for end of transmission.
    //
    // Note: TBF is set by the hardware when the software loads the transmit
    // register (I2C1TRN).  The complexity cannot be reduced by checking 
    // only the state of status bit 'TRSTAT' (to identify reception of 
    // ACK/NAK) since TRSTAT is set high after SDA is updated with the first
    // data byte.  That is, a delay exists between updating the transmit
    // register (I2C1TRN) and status register 'TRSTAT' being '1'.
    //
    while( I2C1STATbits.TBF == 1 );

    // Wait for reception of ACK/NAK.
    while( I2C1STATbits.TRSTAT == 1 );
    
    // NAK is received from slave ?
    if( I2C1STATbits.ACKSTAT == 1 )
    {
        i2c_error_latch = true;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Receive a data byte.
///
/// @return Received data byte.
////////////////////////////////////////////////////////////////////////////////
static uint8_t I2CRxSeq ( void )
{
    uint8_t rx_data;
    
    // Enable Receive sequence
    I2C1CON1bits.RCEN = 1;
    
    // Wait for the data to be received.
    while( I2C1STATbits.RBF == 0 );
    
    // Read the received data.
    rx_data = I2C1RCV;
    
    // Enable an Acknowledge sequence.
    I2C1CON1bits.ACKEN = 1;
    
    // Wait for the Acknowledge sequence to complete.
    while( I2C1CON1bits.ACKEN == 1 );
    
    return rx_data;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Stop an I2C sequence.
////////////////////////////////////////////////////////////////////////////////
static void I2CStopSeq ( void )
{
    // Enable a Stop sequence
    I2C1CON1bits.PEN = 1;
    
    // Wait for Stop sequence to complete.
    while( I2C1CON1bits.PEN == 1 );
}