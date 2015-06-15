

#ifndef PWM_H_
#define PWM_H_

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initialize the PWM module.
/// @param  
/// @return 
////////////////////////////////////////////////////////////////////////////////
void PWMInit ( void );

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enable the PWM module for operation.
/// @param  
/// @return 
////////////////////////////////////////////////////////////////////////////////
void PWMEnable ( void );

////////////////////////////////////////////////////////////////////////////////
/// @brief  Set the PWM duty cycle.
/// @param  pwm_duty - selected duty cycle (note: 1us LSB).
/// @return 
////////////////////////////////////////////////////////////////////////////////
void PWMDutySet ( uint16_t pwm_duty );

#endif  // PWM_H_