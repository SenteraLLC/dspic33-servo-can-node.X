

#ifndef INA219_H
#define	INA219_H


#define INA219_I2C_ADDR     0x40


typedef enum
{
    INA219_REG_CONFIG       = 0x00,     // Configuration Register
    INA219_REG_SHUNT_VOLT   = 0x01,     // Shunt Voltage Register
    INA219_REG_BUS_VOLT     = 0x02,     // Bus Voltage Register
    INA219_REG_POWER        = 0x03,     // Power Register
    INA219_REG_CURRENT      = 0x04,     // Current Register
    INA219_REG_CAL          = 0x05,     // Calibration Register
} INA219_REG_E;


typedef struct {
    union {
        struct {
            unsigned MODE:3;
            unsigned SADC:4;
            unsigned BADC:4;
            unsigned PG:2;
            unsigned BRNG:1;
            unsigned :1;
            unsigned RST:1;
        };
        struct {
            unsigned MODE1:1;
            unsigned MODE2:1;
            unsigned MODE3:1;
            unsigned SADC1:1;
            unsigned SADC2:1;
            unsigned SADC3:1;
            unsigned SADC4:1;
            unsigned BADC1:1;
            unsigned BADC2:1;
            unsigned BADC3:1;
            unsigned BADC4:1;
            unsigned PG0:1;
            unsigned PG1:1;
            unsigned :1;
            unsigned :1;
            unsigned :1;
        };
    };
} INA219_REG_CONFIG_S;


typedef struct {
    union {
                                    // TODO: bit length depends on scale
    };
} INA219_REG_SHUNT_VOLT_S;


typedef struct {
    union {
        struct {
            unsigned OVF:1;
            unsigned CNVR:1;
            unsigned :1;
            unsigned BD:13;         // TODO: bit length depends on scale
        };
        struct {
            unsigned :1;
            unsigned :1;
            unsigned :1;
            unsigned BD0:1;
            unsigned BD1:1;
            unsigned BD2:1;
            unsigned BD3:1;
            unsigned BD4:1;
            unsigned BD5:1;
            unsigned BD6:1;
            unsigned BD7:1;
            unsigned BD8:1;
            unsigned BD9:1;
            unsigned BD10:1;
            unsigned BD11:1;
            unsigned BD12:1;
        };
    };
} INA219_REG_BUS_VOLT_S;


typedef struct {
    union {
        unsigned PD:16;
        struct {
            unsigned PD0:1;
            unsigned PD1:1;
            unsigned PD2:1;
            unsigned PD3:1;
            unsigned PD4:1;
            unsigned PD5:1;
            unsigned PD6:1;
            unsigned PD7:1;
            unsigned PD8:1;
            unsigned PD9:1;
            unsigned PD10:1;
            unsigned PD11:1;
            unsigned PD12:1;
            unsigned PD13:1;
            unsigned PD14:1;
            unsigned PD15:1;
        };
    };
} INA219_REG_POWER_S;


typedef struct {
    union {
        unsigned CD:16;
        struct {
            unsigned CD0:1;
            unsigned CD1:1;
            unsigned CD2:1;
            unsigned CD3:1;
            unsigned CD4:1;
            unsigned CD5:1;
            unsigned CD6:1;
            unsigned CD7:1;
            unsigned CD8:1;
            unsigned CD9:1;
            unsigned CD10:1;
            unsigned CD11:1;
            unsigned CD12:1;
            unsigned CD13:1;
            unsigned CD14:1;
            unsigned CD15:1;
        };
    };
} INA219_REG_CURRENT_S;


typedef struct {
    union {
        unsigned FS:16;
        struct {
            unsigned FS0:1;
            unsigned FS1:1;
            unsigned FS2:1;
            unsigned FS3:1;
            unsigned FS4:1;
            unsigned FS5:1;
            unsigned FS6:1;
            unsigned FS7:1;
            unsigned FS8:1;
            unsigned FS9:1;
            unsigned FS10:1;
            unsigned FS11:1;
            unsigned FS12:1;
            unsigned FS13:1;
            unsigned FS14:1;
            unsigned FS15:1;
        };
    };
} INA219_REG_CAL_S;




// Function Prototypes =========================================================

void INA219Task();


#endif	/* INA219_H */

