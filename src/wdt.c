
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
#include "wdt.h"

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Variable Definitions **********************
// *****************************************************************************

// *****************************************************************************
// ************************** File-Scope Variable Definitions ******************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

// *****************************************************************************
// ************************** Global Functions *********************************
// *****************************************************************************
void WDTService ( void )
{
    // Clear the watchdog timer values.
    //
    // Note: Hardware specific function (see 'xc.h') defined for clearing
    // the watchdog timer values.
    ClrWdt();
}

void WDTEnable ( void )
{
    // Enable watchdog timer operation.
    //
    // Note: enabling of the watchdog timer causes a reset of the counter
    // value.
    //    
    RCONbits.SWDTEN = 1;
}

void WDTDisable ( void )
{
    // Disable watchdog timer operation.
    RCONbits.SWDTEN = 0;    
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************