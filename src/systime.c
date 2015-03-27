

#include <xc.h>
#include <stdint.h>
#include "systime.h"



uint64_t counter = 0;


//==============================================================================

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    counter++;
}

