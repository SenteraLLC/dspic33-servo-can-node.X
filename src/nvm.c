
////////////////////////////////////////////////////////////////////////////////
///
/// @file   $FILE$
/// @author $AUTHOR$
/// @date   $DATE$
/// @brief    
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
#include "nvm.h"
#include "wdt.h"
#include "hw.h"

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
void NVMInit ( void )
{
    NVMCONbits.WREN    = 1;     // Enabled program/erase operations.
    NVMCONbits.NVMSIDL = 0;     // N/A since idle mode not entered.  Set to continue Flash operations when in idle mode for robustness.
    NVMCONbits.RPDF    = 1;     // Data is stored in RAM in an uncompressed format.
}

bool NVMErasePage ( uint16_t erase_addr )
{
    bool nvm_error = false;
    
    NVMCONbits.NVMOP = 0b0011;  // Select a Memory page erase operation.
    
    // Load the address of the erased page.
    //
    // Default memory model is used (i.e. 'small' memory model) - therefore,
    // data is accessed using 16-bit pointer.
    NVMADRU = 0;
    NVMADR  = erase_addr;
    
    // Disable control flow execution.  
    // - interrupts
    // - watchdog timer
    // - timer1
    //
    // This is performed to maintain expected control flow through CPU stall.
    // The CPU stalls until the erase operation is finished. The CPU will 
    // not execute any instructions or respond to interrupts during this time. 
    // If any interrupts occur during the programming cycle, they will remain
    // pending until the cycle completes.
    //
    __builtin_disi( 0x3FFF );
    WDTDisable();
    HwTMRDisable();
    
    // Perform unlock sequence and initiate starting the program/erase cycle.
    //
    // NOTE: The CPU stalls until the erase operation is finished. The CPU will 
    // not execute any instructions or respond to interrupts during this time. 
    // If any interrupts occur during the programming cycle, they will remain
    // pending until the cycle completes.
    //
    NVMKEY = 0x55;
    NVMKEY = 0xAA;
    NVMCONbits.WR = 1;
    
    // Two NOP instructions are required after starting the program/erase cycle.
    __builtin_nop();
    __builtin_nop();
    
    // Wait for the erase cycle to be completed by the hardware
    while( NVMCONbits.WR == 1 );
    
    // Re-enable control flow execution:
    // - interrupts
    // - watchdog timer
    // - timer1
    __builtin_disi( 0 );
    WDTEnable();
    HwTMREnable();
    
    // Identify an NVM erase error if the hardware indicates an improper
    // erase sequence attempted.
    if( NVMCONbits.WRERR == 1 )
    {
        nvm_error = true;
    }
    
    return nvm_error;
}

bool NVMProgramPage ( void* src_data, uint16_t dest_addr )
{
    uint16_t program_row_idx;
    bool     nvm_error = false;
    
    // Typecast src_data pointer to unsigned 16-bit integer type for
    // arithmetic processing.
    src_data = (uint16_t*) src_data;
    
    // Select a Memory row program operation.
    NVMCONbits.NVMOP = 0b0010;  
    
    // Increment through the Program Page (1024 words), one row 
    // (128 words) at a time.
    for( program_row_idx = 0;
         program_row_idx < 8;
         program_row_idx++ )
    {
        // Load the address of the source data into the registers.
        //
        // Note: Default memory model is used (i.e. 'small' memory model); 
        // therefore, data is accessed using 16-bit pointer.
        NVMSRCADRH = 0;
        NVMSRCADRL = (uint16_t) &src_data;

        // Load the NVM destination address.
        //
        // Default memory model is used (i.e. 'small' memory model) - therefore,
        // data is accessed using 16-bit pointer.
        NVMADRU = 0;
        NVMADR  = dest_addr;
        
        // Disable control flow execution.  
        // - interrupts
        // - watchdog timer
        // - timer1
        //
        // This is performed to maintain expected control flow through CPU stall.
        // The CPU stalls until the erase operation is finished. The CPU will 
        // not execute any instructions or respond to interrupts during this time. 
        // If any interrupts occur during the programming cycle, they will remain
        // pending until the cycle completes.
        //
        __builtin_disi( 0x3FFF );
        WDTDisable();
        HwTMRDisable();

        // Perform unlock sequence and initiate starting the program/erase cycle.
        //
        // NOTE: The CPU stalls until the erase operation is finished. The CPU will 
        // not execute any instructions or respond to interrupts during this time. 
        // If any interrupts occur during the programming cycle, they will remain
        // pending until the cycle completes.
        //
        NVMKEY = 0x55;
        NVMKEY = 0xAA;
        NVMCONbits.WR = 1;

        // Two NOP instructions are required after starting the program/erase cycle.
        __builtin_nop();
        __builtin_nop();
        
        // Wait for the write cycle to be completed by the hardware
        while( NVMCONbits.WR == 1 );
        
        // Re-enable control flow execution:
        // - interrupts
        // - watchdog timer
        // - timer1
        __builtin_disi( 0 );
        WDTEnable();
        HwTMREnable();
        
        // Increment the source pointer by the program row size (128 words).
        src_data += 128;
    }
    
    // Identify an NVM erase error if the hardware indicates an improper
    // erase sequence attempted, or a program row underrun error occurs.
    if( ( NVMCONbits.WRERR == 1 ) ||
        ( NVMCONbits.URERR == 1 ) )
    {
        nvm_error = true;
    }
    
    return nvm_error;
}














// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief 
/// @param 
/// @return
////////////////////////////////////////////////////////////////////////////////