////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief  Header file for defining hardware operation.   
///
////////////////////////////////////////////////////////////////////////////////

#ifndef HW_H_
#define	HW_H_


void HwInit( void );
void HwTMREnable ( void );
void HwTMRDisable ( void );
void HwWDTService ( void );

void HwTMR1Service ( void );
void HwTMR2Service ( void );
uint16_t HwTMRp1msGet ( void );

#endif	// HW_H_