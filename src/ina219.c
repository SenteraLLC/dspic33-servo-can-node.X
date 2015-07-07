////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief External current/power monitor (INA219) driver. 
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

// *****************************************************************************
// ************************** User Include Files *******************************
// *****************************************************************************

#include "ina219.h"
#include "i2c.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

/// INA219 slave address.
///
/// @note   INA219 address lines A1 and A0 are electrically grounded.  This 
///         causes the INA219 slave address to be 0b100_0000.
#define INA219_SADDR  0x40U

#define INA219_REG_CFG          0x00     ///< Configuration Register Address
#define INA219_REG_BUS_VOLT     0x02     ///< Bus Voltage Register Address
#define INA219_REG_CURRENT      0x04     ///< Current Register Address
#define INA219_REG_CAL          0x05     ///< Calibration Register Address

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

/// INA219 measured current.
static uint16_t ina219_amp;

/// INA219 measured voltage.
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
    // Note: With 12-bit resolution and a 16V full scale range for the bus
    // voltage (Vin-), the ATD LSB is: 16V / ( 2 ^ 12 ) ~= 4mV.
    //
    // Note: With 12-bit resolution and 80mV positive range for the shunt
    // voltage, the ATD LSB is: 80mV / ( 2 ^ 12 ) ~= 20uV.
    //
    static const uint8_t cfg_reg_data[] = 
    {
        INA219_REG_CFG,
        0b00001101,
        0b01010111,
    };
    
    // INA219 Calibration register data definition:
    //  - byte 1: Calibration register address.
    //  - byte 2: Calibration register MSB value.
    //  - byte 3: Calibration register LSB value.
    //
    // INA219 datasheet Calibration register calculation:
    // (1) Vbus_max     = 10V
    //     Vshunt_max   = 80mV
    //     Rshunt       = 8mOhms
    //
    // (2) MaxPossible_I = 80mV / 8mOhms = 10A
    //
    // (3) Max_Expected_I, chosen as MaxPossible_I (10A).
    //
    // (4) Min_LSB = 10A / 2^15 ~= 3.0E-4
    //     Max_LSB = 10A / 2^12 ~= 2.4E-3
    //     Current_LSB chosen as 1mA
    //
    // (5) Cal = trunc( 0.04096 / ( Current_LSB * Rshunt ) ) = 5120 = 0x1400
    //
    static const uint8_t cal_reg_data[] = 
    {
        INA219_REG_CAL,
        0x14,
        0x00,
    };
    
    // Program the INA219 Configuration register.
    I2CWrite( INA219_SADDR, &cfg_reg_data[ 0 ], sizeof( cfg_reg_data ) );
    
    // Program the INA219 Calibration register.
    I2CWrite( INA219_SADDR, &cal_reg_data[ 0 ], sizeof( cal_reg_data ) );    
}

void INA219Service ( void )
{
    static const uint8_t volt_sel_data[] = 
    {
        INA219_REG_BUS_VOLT,
    };
    
    static const uint8_t amp_sel_data[] = 
    {
        INA219_REG_CURRENT,
    };
    
    uint16_t volt_reg_val;
    int16_t  amp_reg_val;
    
    // Select the Bus Voltage register for the subsequent read operation.
    I2CWrite( INA219_SADDR, &volt_sel_data[ 0 ], sizeof( volt_sel_data ) );
    
    // Read the Bus Voltage (Vin-) value.
    I2CRead( INA219_SADDR, (uint8_t*) &volt_reg_val, sizeof ( volt_reg_val ) );
    
    // 1. Remove Bus Voltage offset - within the INA219 register, the value
    // is positioned at bits 14-3.
    //
    // 2. Scale the Bus Voltage to an LSb of 1mV.  With the peripheral's
    // configuration (see initialization function), the values scaling is 
    // an LSb of 4mV.  Therefore, the value need to be multiplied by 4.
    //
    ina219_volt = volt_reg_val >> 3;
    ina219_volt = ina219_volt  << 2;
    
    // Select the Current register for the subsequent read operation.
    I2CWrite( INA219_SADDR, &amp_sel_data[ 0 ], sizeof( amp_sel_data ) );
    
    // Read the Current value.
    I2CRead( INA219_SADDR, (uint8_t*) &amp_reg_val, sizeof ( amp_reg_val ) );
    
    // Saturate current to a positive value.  The INA219 current register is a
    // signed value, but negative current is not expected.
    if ( amp_reg_val < 0 )
    {
        amp_reg_val = 0;
    }
    
    ina219_amp = amp_reg_val;
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