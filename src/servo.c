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
#include "servo.h"
#include "ina219.h"
#include "can.h"
#include "cfg.h"
#include "util.h"
#include "pwm.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************
static uint16_t servo_cmd_type;
static uint16_t servo_cmd_pwm;
static int16_t  servo_cmd_pos;
static uint16_t servo_act_pwm;

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
        
        // Perform correction of position commanded value.
        // servo_act_pwm = UtilPolyMul( servo_cmd_pos, &servo_coeff[ 0 ], CFG_PWM_COEFF_LEN );  COMMENTED OUT FOR DEBUGGING
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
