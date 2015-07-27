////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief Non-volatile Memory (NVM) driver.
////////////////////////////////////////////////////////////////////////////////

#ifndef NVM_H_
#define	NVM_H_

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

// *****************************************************************************
// ************************** Defines ******************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Declarations *************************************
// *****************************************************************************

// *****************************************************************************
// ************************** Function Prototypes ******************************
// *****************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// @brief Initialize NVM hardware.
////////////////////////////////////////////////////////////////////////////////
void NVMInit ( void );

////////////////////////////////////////////////////////////////////////////////
/// @brief  Erase a page of NVM.
///
/// @param  table_page
///             The table page number for the NVM page being erased.  Can be
///             determined using built-in function "__builtin_tblpage".
/// @param  table_offset
///             The table page offset for the NVM page being erased.  Can be
///             determined using built-in function "__builtin_tbloffset".
///
/// @return true  - error in NVM page erase operation.
///         false - NVM page erase was successful.
////////////////////////////////////////////////////////////////////////////////
bool NVMErasePage ( uint16_t table_page, 
                    uint16_t table_offset );

////////////////////////////////////////////////////////////////////////////////
/// @brief  Program (i.e. write) a page of NVM.
///
/// @param  src_data
///             Data to be written to the NVM page.
/// @param  table_page
///             The table page number for the NVM page being programmed.  Can be
///             determined using built-in function "__builtin_tblpage".
/// @param  table_offset
///             The table page offset for the NVM page being programmed.  Can be
///             determined using built-in function "__builtin_tbloffset".
///
/// @return true  - error in NVM page program operation.
///         false - NVM page program was successful.
////////////////////////////////////////////////////////////////////////////////
bool NVMProgramPage ( const uint16_t src_data[],
                      uint16_t table_page, 
                      uint16_t table_offset );

#endif	// NVM_H_