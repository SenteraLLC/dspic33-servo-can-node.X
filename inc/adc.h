////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief  Header file for defining hardware operation.   
///
////////////////////////////////////////////////////////////////////////////////

#ifndef ADC_H_
#define	ADC_H_

typedef enum
{
    ADC_AIN2,
    ADC_AIN3,
    
    ADC_AIN_MAX
    
} ADC_AIN_E;


void ADCInit ( void );
void ADCService ( void );
uint16_t ADCGet ( ADC_AIN_E adc_sel );

#endif	// ADC_H_