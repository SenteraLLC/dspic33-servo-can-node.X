

#include <xc.h>
#include <stdint.h>
#include "ina219.h"

void INA219Task()
{
    static int state = 1;
    
    switch (state)
    {
        case 1:
        {
            //    1. Assert a Start condition on SDAx and SCLx.
            I2C1CON1bits.SEN = 1;   // Enable start condition.
            state++;
            break;
        }
        case 2:
        {
            //    2. Send the I2C device address byte to the slave with a write indication.
            if (I2C1CON1bits.SEN == 0)
            {
//                I2C1TRN = INA219_I2C_ADDR << 1;             // Write.
                I2C1TRN = (INA219_I2C_ADDR << 1) | 0x01;    // Read.
                state++;
            }
            break;
        }
        case 3:
        {
            //    3. Wait for and verify an Acknowledge from the slave.
            if (I2C1STATbits.TRSTAT == 0)
            {
                if (I2C1STATbits.ACKSTAT == 0)  // Wait for ACK.
                {
//                    state++;
                    state = 6;
                }
                else
                {
                    // TODO: ack was not received from the slave!
                    Nop();
                }
            }
            break;
        }
        case 4:
        {
            //    4. Send the serial memory address high byte to the slave.
            I2C1TRN = 0x00;
            state++;
            break;
        }
        case 5:
        {
            //    5. Wait for and verify an Acknowledge from the slave.
            if (I2C1STATbits.TRSTAT == 0)
            {
                if (I2C1STATbits.ACKSTAT == 0)  // Wait for ACK.
                {
                    state++;
                }
                else
                {
                    // TODO: ack was not received from the slave!
                    Nop();
                }
            }
            break;
        }
        case 6:
        {
            //    11. Enable the master reception to receive serial memory data.
            I2C1CON1bits.RCEN = 1;
            state++;
            break;
        }
        case 7:
        {
            //    12. Generate an ACK or NACK condition at the end of a received byte of data.
            if (I2C1CON1bits.RCEN == 0)
            {
                I2C1CON1bits.ACKEN = 1;     // Acknowledge Sequence Enable.
                state++;
            }
            break;
        }
        case 8:
        {
            if (I2C1CON1bits.ACKEN == 0)
            {
                I2C1CON1bits.RCEN = 1;
                state++;
            }
            break;
        }
        case 9:
        {
            if (I2C1CON1bits.RCEN == 0)
            {
                Nop();
            }
        }
    }
}


//==============================================================================

//int INA219WriteReg(INA219_REG_E reg, uint16_t data)
//{
//    
//}

//==============================================================================

//int INA219ReadReg(INA219_REG_E reg, uint8_t *data)
//{
//    
//}


//==============================================================================

void __attribute__((__interrupt__, no_auto_psv)) _I2C1Interrupt(void)
{
    
}

// ? Start condition
// ? Stop condition
// ? Data transfer byte transmitted or received
// ? Acknowledge transmit
// ? Repeated Start
// ? Detection of a bus collision event





