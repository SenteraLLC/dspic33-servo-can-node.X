

#include <xc.h>
#include "init.h"


//==============================================================================

static void InitCPU()
{
    // Initialize oscillator.
    
    // Fosc = Fin * (M / (N1 * N2))         |   N1 = PLLPRE + 2
    //      = 10 MHz * (32 / (2 * 4))       |   N2 = 2 * (PLLPOST + 1))
    //      = 40 MHz                        |   M = PLLDIV + 2
    //
    // 40 MHz = 20 MIPS
    
    CLKDIVbits.PLLPRE = 0;          // PLL input divided by 2.      (N1)
    CLKDIVbits.PLLPOST = 1;         // PLL output divided by 4.     (N2)
    PLLFBDbits.PLLDIV = 30;         // PLL feedback divisor is 32.  (M)
    
    while (OSCCONbits.LOCK == 0);   // Wait for the PLL to lock.
}

//==============================================================================

static void InitPPS()
{
    // Initialize peripheral pin select.
    
    // RP20 = C1RX
    // RP36 = C1TX
    
    // RP37 = SDA1
    // RP38 = SCL1
    
    // RP42 = PWM1H3 = PWM-PPM_IN
    // RP43 = PWM1L3 = PWM_OUT
    
    
}

//==============================================================================

static void InitI2C()
{
    // Initialize I2C1
    
    
}

//==============================================================================

static void InitCAN()
{
    // Initialize CAN1
    
    
}

//==============================================================================

static void InitTMR()
{
    // Timer 1 (system timer)
    
    // NOTE: timer is fed by instruction clock!
    
    
    T1CON = 0x00;               // Clear Timer1 Control Register.
    T1CONbits.TCKPS = 0b11;     // Prescale = 1:256.
    
    IPC0bits.T1IP = 6;          // Interrupt Priority 6.
    IFS0bits.T1IF = 0;          // Clear interrupt flag.
    IEC0bits.T1IE = 1;          // Enable interrupts.
    
    T1CONbits.TON = 1;          // Start 16-bit Timer1.
}

//==============================================================================

static void InitINT()
{
    // Initialize interrupts.
    
    INTCON2bits.GIE = 1;        // Global Interrupt Enable
}

//==============================================================================

void InitBoard()
{
    // Initialize dsPIC33 peripherals.

    InitCPU();
    InitPPS();
    InitI2C();
    InitCAN();
    InitTMR();
    InitINT();
}

