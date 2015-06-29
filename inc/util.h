////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief  Header file for defining hardware operation.   
///
////////////////////////////////////////////////////////////////////////////////

#ifndef UTIL_H_
#define	UTIL_H_

int32_t UtilPoly32( int32_t var_in,
                    uint8_t calc_qnum,
                    int32_t coeff[], 
                    uint8_t coeff_len );

void UtilDelay( uint16_t ms_time );

#endif	// UTIL_H_