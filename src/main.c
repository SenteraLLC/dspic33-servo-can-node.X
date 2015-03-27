

#include <xc.h>
#include "cfgbits.h"
#include "init.h"


//==============================================================================

int main()
{
    InitBoard();        // Initialize processor & peripherals.
    
    
    for (;;)
    {
        asm("CLRWDT");      // Clear the watchdog timer.
    }
    
    return 0;
}

