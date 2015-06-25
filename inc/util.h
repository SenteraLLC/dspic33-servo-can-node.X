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

int64_t UtilPoly( int64_t  var_in,
                  uint64_t var_mul,
                  int32_t  coeff[], 
                  uint8_t  coeff_len );

void UtilDelay( uint16_t ms_time );

#endif	// UTIL_H_