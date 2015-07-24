////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Servo management.
////////////////////////////////////////////////////////////////////////////////

// *****************************************************************************
// ************************** System Include Files *****************************
// *****************************************************************************

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
// - PWM output scale       = 1E6 - ( i.e. LSB = 1.0   us       ).
//
// Rationale:
//  The polynomial equation is implemented using fixed-point math since the 
//  hardware does not natively support floating-point arithmetic and the 
//  execution time of a floating-point implementation exceeds that available
//  for the calculation resolution required (i.e. double-precision).
//
//  Additionally, base2 fixed-point math is used rather than base10 as 
//  64-bit division is required and the time required for a base10 
//  implementation (i.e. integer division rather than bit-shift) exceeds
//  the processing time available.
//
// -----------------------------------------------------------------------------
//
// SERVO_QNUM_CALC:
//  The servo position command is up-scaled for resolution on internal 
//  calculation.  This is critical for maintained accuracy through the power
//  terms (e.g. pos^5) of the polynomial equation.
//
// SERVO_PWM_DIV:
//  The output of the polynomial equation has a scaling based on the 
//  coefficient scaling (i.e. 1E8).  The output is  down-scaled to remove the 
//  input scaling, and get the result term to the required units 
//  (i.e. 1E8 / 1E6 = 1E2).
//
// -----------------------------------------------------------------------------
//
#define SERVO_QNUM_CALC      30U        ///< Polynomial calculation Q-number.
#define SERVO_PWM_OUT_DIV   100U        ///< Post-calculation division factor.

// Values for scaling the PWM input to Q30 representation.
#define SERVO_PWM_IN_SHIFT1       21U   ///< Input position 1st l-shift (for Q30 scaling).
#define SERVO_PWM_IN_DIV        1000U   ///< Input position division (for Q30 scaling).
#define SERVO_PWM_IN_SHIFT2        9U   ///< Input position 2nd l-shift (for Q30 scaling).

/// List of servo control/command types.
typedef enum
{
    SERVO_CTRL_PWM,     ///< Servo controlled with PWM command.
    SERVO_CTRL_POS      ///< Servo controlled with position command.
    
} SERVO_CTRL_TYPE_E;

// *****************************************************************************
// ************************** Definitions **************************************
// *****************************************************************************

/// The command type used to control the servo.
///
/// @note   Default to PWM control.
static SERVO_CTRL_TYPE_E servo_cmd_type = SERVO_CTRL_PWM;

/// The PWM commanded value for servo control.
///
/// @note   Default to 1500us - i.e. neutral position.
static uint16_t servo_cmd_pwm = 1500;

/// The position commanded value for servo control.
///
/// @note   Default to 0.0 radians.
static int16_t servo_cmd_pos = 0;

/// The actual PWM value applied to the servo.
///
/// @note   Updated on execution of module service function.
static uint16_t servo_act_pwm;

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************

void ServoService ( void )
{
    // CAN message transmitted ever software cycle (10ms).
    static const uint16_t can_tx_period  = 1;
    static       uint16_t can_tx_timeout = 0;
    
    CAN_RX_SERVO_CMD_U    servo_cmd_msg;
    CAN_TX_SERVO_STATUS_U servo_status_msg;
    
    int32_t servo_coeff[ CFG_PWM_COEFF_LEN ];
    
    int32_t servo_cmd_pos_in;
    int32_t servo_act_pwm_i32;
    
    bool payload_valid;
    
    // Get the Servo Command CAN data.
    payload_valid = CANRxGet( CAN_RX_MSG_SERVO_CMD, servo_cmd_msg.data_u16 );
    
    // Servo Command CAN message received ?
    if( payload_valid == true )
    {
        // Update module data with that received.
        servo_cmd_type = servo_cmd_msg.cmd_type;
        servo_cmd_pwm  = servo_cmd_msg.cmd_pwm;
        servo_cmd_pos  = servo_cmd_msg.cmd_pos;
    }
    
    // Position command is being used for control ?
    if( servo_cmd_type == SERVO_CTRL_POS )
    {
        // Get servo polynomial coefficient correction values.
        CfgPWMCoeffGet( &servo_coeff[ 0 ] );
        
        // Note: Implementation defined behavior.  Shift operators retain the sign.
        servo_cmd_pos_in = ((int32_t) servo_cmd_pos) << SERVO_PWM_IN_SHIFT1;    // up-scale to maximize storage in int32_t.
        servo_cmd_pos_in = servo_cmd_pos_in / SERVO_PWM_IN_DIV;                 // remove base_10 scaling.
        servo_cmd_pos_in = servo_cmd_pos_in << SERVO_PWM_IN_SHIFT2;             // up-scale to Q30 representation.
        
        // Perform correction of position commanded value.
        servo_act_pwm_i32 = UtilPoly32( servo_cmd_pos_in,
                                        SERVO_QNUM_CALC,
                                        &servo_coeff[ 0 ], 
                                        CFG_PWM_COEFF_LEN );
        
        // Down-scale and typecast value back to integer type (micro-sec LSB).
        servo_act_pwm = (uint16_t) ( servo_act_pwm_i32 / SERVO_PWM_OUT_DIV );
    }
    else
    {
        // Commanded PWM is used directly for control.
        servo_act_pwm = servo_cmd_pwm;                                       
    }
    
    // Update PWM duty cycle with that determined.
    PWMDutySet( servo_act_pwm );
    
    // timeout has elapsed since last CAN message transmission.
    can_tx_timeout++;
    if( can_tx_timeout >= can_tx_period )
    {
        // reset timeout
        can_tx_timeout = 0;
        
        // Construct the Servo Status CAN message.
        servo_status_msg.cmd_type_echo = servo_cmd_type;
        servo_status_msg.pwm_act       = servo_act_pwm;
        servo_status_msg.servo_voltage = INA219VoltGet();
        servo_status_msg.servo_current = INA219AmpGet();

        // Send the CAN message.
        CANTxSet( CAN_TX_MSG_SERVO_STATUS, servo_status_msg.data_u16 );
    }
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************
