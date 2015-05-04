

#include <xc.h>
#include "cfgbits.h"
#include "init.h"
#include "ina219.h"


//==============================================================================

int main()
{
    InitBoard();        // Initialize processor & peripherals.
    
    for (;;)
    {
        asm("CLRWDT");              // Clear the watchdog timer.
        INA219Task();               // Current and power monitor.
    }
    
    return 0;
}

