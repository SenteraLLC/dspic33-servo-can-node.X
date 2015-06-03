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
#include "ina219.h"
#include "i2c.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// Slave address read and write commands.
//
// Note: INA219
#define INA219_SADDR_READ  0x10U
#define INA219_SADDR_WRITE 0x11U

#define INA219_REG_ADDR_CFG          0x00     // Configuration Register
#define INA219_REG_ADDR_SHUNT_VOLT   0x01     // Shunt Voltage Register
#define INA219_REG_ADDR_BUS_VOLT     0x02     // Bus Voltage Register
#define INA219_REG_ADDR_POWER        0x03     // Power Register
#define INA219_REG_ADDR_CURRENT      0x04     // Current Register
#define INA219_REG_ADDR_CAL          0x05     // Calibration Register

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************
static uint16_t ina219_amp;
static uint16_t ina219_volt;

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
void INA219Init ( void )
{
    // INA219 Configuration register data definition:
    //  - byte 1: Configuration register address.
    //  - byte 2: Configuration register MSB value.
    //  - byte 3: Configuration register LSB value.
    //
    // Configuration register value:
    //  - RST:   bits    15, 0b0    = peripheral reset is not performed.
    //  - Spare: bits    14, 0b0
    //  - BRNG:  bits    13, 0b0    = 16V full scale range is used.  Measured bus voltage (i.e. Vin-) max value expected is less than 10V.
    //  - PG:    bits 12-11, 0b01   = Shunt voltage range of +-80mV used. At 10A shunt current (max) the shunt voltage is 80mV.
    //  - BADC   bits 10- 7, 0b1010 = Bus voltage sampled with 12-bit resolution and 4 sample averaging.  2.13ms conversion time.
    //  - SADC   bits  6- 3, 0b1010 = Shunt voltage sampled with 12-bit resolution and 4 sample averaging.  2.13ms conversion time.
    //  - MODE   bits  2- 0, 0b111  = Shunt voltage and Bus voltage continuously sampled.
    //
    static const uint8_t cfg_data[] = 
    {
        INA219_REG_ADDR_CFG,
        0b00001101,
        0b01010111,
    };
    
    // Program the INA210 Configuration register.
    I2CWrite( INA219_SADDR_WRITE, &cfg_data[ 0 ], sizeof( cfg_data ) );
}

void INA219Service ( void )
{
    // DEBUG CODE (START): Read the configuration register value to verify its setting.
    static uint8_t cfg_data_read[ 2 ];
    
    I2CRead( INA219_SADDR_READ, &cfg_data_read[ 0 ], 2U );
    // DEBUG CODE (END)
}

uint16_t INA219AmpGet ( void )
{
    return ina219_amp;
}

uint16_t INA219VoltGet ( void )
{
    return ina219_volt;
}
        
// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief  
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////