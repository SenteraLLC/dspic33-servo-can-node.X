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
#include "servo.h"
#include "ina219.h"
#include "can.h"
#include "cfg.h"
#include "util.h"
#include "pwm.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// Scaling factors for servo correction polynomial fields:
//
// - Coefficient scale      = 1E8 - ( i.e. LSB = 0.01  us/rad^x ).
// - PWM input scale        = 1E3 - ( i.e. LSB = 0.001 rad      ).
// - PWM calculation scale  = 1E9 - ( i.e. LSB = 1.0   nano-rad ).
// - PWM output scale       = 1E6 - ( i.e. LSB = 1.0   us       ).
//
// Rationale:
//  The polynomial equation is implemented using fixed-point math since the 
//  hardware does not natively support floating-point arithmetic and the 
//  execution time of a floating-point implementation exceeds that available
//  for the calculation resolution required (i.e. double-precision).
//
// -----------------------------------------------------------------------------
//
// Coefficient scale:
//  Input polynomial coefficients scaling is expected to be 1E8.
//
// PWM input scale:
//  Input servo position command scaling is expected to be 1E3.
//  
// PWM calculation scale
//  The servo position command is up-scaled for resolution on internal 
//  calculation.  This is critical for maintained accuracy through the power
//  terms (e.g. pos^5) of the polynomial equation.
//
// PWM output scale:
//  The output of the polynomial equation has a scaling based to the 
//  coefficients and variable scaling (i.e. 1E8 * 1E9).  The output is 
//  down-scaled to remove the input scaling, and get the result term to the 
//  required units (i.e. micro-seconds, 1E6).
//
// -----------------------------------------------------------------------------
//
// Dynamic Range:
//  With a PWM input dynamic range of [-1.000:1.000] radians, the
//  calculation scaled value spans [-1E9:1E9].  With coefficient values 
//  having a storage size of int32_t (i.e. [-2.15E9:2.15E9]) the maximum
//  polynomial result achievable (with a 5th degree polynomial) is 
//  approximately [-1.2E19:1.2E19].  This value is just outside the 
//  storage size for int64_t, but is used as it yields a significant
//  calculation resolution and risk of overflow is essentially
//  non-existent.
//
//#define SERVO_COEFF_IN_SCALE     100000000ULL
//#define SERVO_PWM_IN_SCALE            1000ULL
//#define SERVO_PWM_CALC_SCALE    1000000000ULL
//#define SERVO_PWM_OUT_SCALE        1000000ULL
//
//#define SERVO_CALC_IN_MUL      (SERVO_PWM_CALC_SCALE/SERVO_PWM_IN_SCALE)
//#define SERVO_CALC_OUT_DIV     ((SERVO_COEFF_IN_SCALE*SERVO_PWM_CALC_SCALE)/SERVO_PWM_OUT_SCALE)

//#define SERVO_SCALE_POS     3U
//#define SERVO_SCALE_COEFF   8U
//#define SERVO_SCALE_PWM     6U
//#define SERVO_PWM_DIV_ORDER ((SERVO_SCALE_COEFF+SERVO_SCALE_POS)-SERVO_SCALE_PWM)

// The result of the polynomial equation needs to be divided to the expected scale (i.e. us)
#define SERVO_PWM_DIV       100U

// #define SERVO_SCALE_RAW    1000U
#define SERVO_QNUM_CALC      30U
// #define SERVO_QNUM_COR       15U



// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************
static uint16_t servo_cmd_type = 0;     // Default to PWM command type.
static uint16_t servo_cmd_pwm  = 1500;  // Default to 1500us command pulse.
static int16_t  servo_cmd_pos  = 0;     // Default to 0.0 radian position.
static uint16_t servo_act_pwm  = 1500;  // Default to 1500us pulse.

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************
void ServoService ( void )
{
    CAN_TX_SERVO_CMD_U    servo_cmd_msg;
    CAN_TX_SERVO_STATUS_U servo_status_msg;
    
    uint16_t servo_amperage;
    uint16_t servo_voltage;
    
    int32_t servo_coeff[ CFG_PWM_COEFF_LEN ];
    
    int32_t servo_cmd_pos_in;
    int32_t servo_act_pwm_i32;
    
    bool payload_valid;
    
    // Get servo amperage and voltage.
    servo_amperage = INA219AmpGet();
    servo_voltage  = INA219VoltGet();
    
    // Get the Servo Command CAN data.
    payload_valid =  CANRxGet ( CAN_RX_MSG_SERVO_CMD, servo_cmd_msg.data_u16 );
    
    // Servo Command CAN message received ?
    if( payload_valid == true )
    {
        // Update module data with that received.
        servo_cmd_type = servo_cmd_msg.cmd_type;
        servo_cmd_pwm  = servo_cmd_msg.cmd_pwm;
        servo_cmd_pos  = servo_cmd_msg.cmd_pos;
    }
    
    // Position command is being used for control ?
    if( servo_cmd_type == 1 )
    {
        // Get servo polynomial coefficient correction values.
        CfgPWMCoeffGet( &servo_coeff[ 0 ] );
        
        // Note: Implementation defined behavior.  Shift operators retain the sign.
        servo_cmd_pos_in = ((int32_t) servo_cmd_pos) << 21; // up-scale to maximize storage in int32_t.
        servo_cmd_pos_in = servo_cmd_pos_in / 1000;         // remove base_10 scaling.
        servo_cmd_pos_in = servo_cmd_pos_in << 9;           // up-scale to Q30 representation.
        
        // Perform correction of position commanded value.
        servo_act_pwm_i32 = UtilPoly32( servo_cmd_pos_in,
                                        SERVO_QNUM_CALC,
                                        &servo_coeff[ 0 ], 
                                        CFG_PWM_COEFF_LEN );
        
        // Down-scale and typecast value back to integer type (micro-sec LSB).
        servo_act_pwm = (uint16_t) ( servo_act_pwm_i32 / SERVO_PWM_DIV );
    }
    else
    {
        // Commanded PWM is used directly for control.
        servo_act_pwm = servo_cmd_pwm;                                       
    }
    
    // Update PWM duty cycle with that determined.
    PWMDutySet( servo_act_pwm );

    // Construct the Servo Status CAN message.
    servo_status_msg.cmd_type_echo = servo_cmd_msg.cmd_type;
    servo_status_msg.pwm_act       = servo_act_pwm;
    servo_status_msg.servo_voltage = servo_voltage;
    servo_status_msg.servo_current = servo_amperage;
    
    // Send the CAN message.
    CANTxSet ( CAN_TX_MSG_SERVO_STATUS, servo_status_msg.data_u16 );
}

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
