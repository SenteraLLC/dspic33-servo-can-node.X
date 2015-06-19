
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
    NVMCONbits.NVMSIDL = 0;     // N/A, since idle mode not entered.  Set to continue Flash operations when in idle mode for robustness.
    NVMCONbits.RPDF    = 0;     // N/A, since row programming no used.
}

bool NVMErasePage ( uint16_t table_page, 
                    uint16_t table_offset )
{
    bool nvm_error = false;
    
    // Load the address of the erased page.
    NVMADRU = table_page;
    NVMADR  = table_offset;
    
    // Select a Memory page erase operation.
    NVMCONbits.NVMOP = 0b0011;
    
    // Disable control flow execution.  
    // - interrupts
    // - watchdog timer
    // - timer1
    //
    // This is performed to maintain expected control flow through CPU stall.
    // The CPU stalls until the erase operation is finished. The CPU will 
    // not execute any instructions or respond to interrupts during this time. 
    // If any interrupts occur during the erase cycle, they will remain pending 
    // until the cycle completes.
    //
    WDTDisable();
    HwTMRDisable();
    __builtin_disi( 0x3FFF );
    
    // Perform unlock sequence and initiate starting the erase cycle.
    NVMKEY = 0x55;
    NVMKEY = 0xAA;
    NVMCONbits.WR = 1;
    
    // Two NOP instructions are required after starting the erase cycle.
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

bool NVMProgramPage ( const uint16_t src_data[],
                            uint16_t dest_table_page, 
                            uint16_t dest_table_offset )
{
    uint16_t  instr_idx;
    bool      nvm_error = false;
    
    uint16_t tblpag_store;
    
    // Increment through the Program Memory until a Program Page is programmed
    // (i.e. written).
    //
    // Note: Each Program Page is composed of 1024 words. Program Memory is 
    // organized into even-addressed instruction (aka double-word) elements 
    // (to coincide with the  Data Memory architecture).  The most-significant 
    // byte (3rd byte) of an instruction element is the 'phantom' byte which is 
    // not writable.  The next byte (2nd byte) is not used for data storage 
    // (i.e. __pack_upper_byte compiler option not used).  The two least-
    // significant bytes (1-0 bytes) are used for data storage.
    //
    // Program Memory Architecture:
    //  Address     byte3       byte2       byte1       byte0
    //  0           'phantom'   0           dd          dd
    //  2           'phantom'   0           dd          dd
    //  4           'phantom'   0           dd          dd
    //  :           :           :           :           :
    //  :           :           :           :           :
    //
    // Program Memory is written in two instructions (i.e. 4 words) at a time.
    // Therefore, the number of write operations (i.e. loop cycles) is:
    // 512 / 2 = 256.
    //
    for( instr_idx  = 0;
         instr_idx  < 512;
         instr_idx += 2 )
    {
        // Load the NVM destination address.
        //
        // Note: For the lower address (NVMADR), the instruction index is
        // multiplied by 2 since each instruction occupies two addressable
        // units within the Program Memory architecture.
        //
        NVMADRU = dest_table_page;
        NVMADR  = dest_table_offset + ( instr_idx * 2 );
        
        // Load the two instructions into the latches.
        //
        // Note: Write Latches are contained in Program Memory address
        // 0xFA0000-0xFA0003 (i.e. 2 instructions).  This requires updating 
        // TBLPAG to access the latches (i.e. TBLPAG is the upper 8-bits for 
        // the address).
        //
        // Note: The first argument to __builin_tblwt* is the latch table 
        // 'offset'.  Program Memory is organized in double-words; therefore an
        // 'offset' value of '0' and '1' are equivalent.  That is, both of the
        // following instructions access the LSW (i.e. the lower word of the
        // double-word) of the 1st double-word element:
        //      ->  ____builtin_tblwtl( 0 , 0 );
        //      ->  ____builtin_tblwtl( 1 , 0 );
        //
        // Note: The Table Page register (TBLPAG) is restored to its previous
        // value following modification.  This is performed as a best-practice
        // to not corrupt other function's accesses using the Table Page
        // register.
        //
        // Note: Since only the lower-word of an instruction is used for 
        // storing of data (i.e. __pack_upper_byte compiler option not used)
        // the higher-word of the instruction is cleared.
        //
        tblpag_store = TBLPAG;
        TBLPAG = 0xFA;
        
        __builtin_tblwtl( 0 , src_data[ instr_idx ]     );
        __builtin_tblwth( 0 , 0                         );      
        __builtin_tblwtl( 2 , src_data[ instr_idx + 1 ] );
        __builtin_tblwth( 2 , 0                         );
        
        TBLPAG = tblpag_store;
        
        // Select a Memory word program operation.
        NVMCONbits.NVMOP = 0b0001;  
        
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
        WDTDisable();
        HwTMRDisable();
        __builtin_disi( 0x3FFF );

        // Perform unlock sequence and initiate starting the program cycle.
        NVMKEY = 0x55;
        NVMKEY = 0xAA;
        NVMCONbits.WR = 1;

        // Two NOP instructions are required after starting the program cycle.
        __builtin_nop();
        __builtin_nop();
        
        // Wait for the program cycle to be completed by the hardware
        while( NVMCONbits.WR == 1 );
        
        // Re-enable control flow execution:
        // - interrupts
        // - watchdog timer
        // - timer1
        __builtin_disi( 0 );
        WDTEnable();
        HwTMREnable();
        
        // Identify an NVM program error if the hardware indicates an improper
        // program sequence attempted.
        if( NVMCONbits.WRERR == 1 )
        {
            nvm_error = true;
        }
    }
    
    return nvm_error;
}

// *****************************************************************************
// ************************** Static Functions *********************************
// *****************************************************************************