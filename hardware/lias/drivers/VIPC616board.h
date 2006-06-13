// ============================================================================
//
// = KERNEL_MODULE
//    VIPC616board_module.o
//
// = FILENAME
//    VIPC616board.h
//
// = DESCRIPTION
//    Module to implement the interface between the IP-modules and the 
//    vme-rack
//
// = AUTHOR
//    Rene Waarsing <Rene.Waarsing@mech.kuleuven.ac.be>
//
// = CREDITS
//    Based on work from Johan Loeckx
//
// ============================================================================
//
// $Id: VIPC616board.h,v 1.1.2.3 2004/06/28 15:34:43 rwaarsin Exp $
// $Log: VIPC616board.h,v $
// Revision 1.1.2.3  2004/06/28 15:34:43  rwaarsin
// Added docs
//
// Revision 1.1.2.2  2003/08/18 16:07:22  rwaarsin
// Changed to new version of kernel, thus a new version of the Universe module
//
//
// ============================================================================

#ifndef VIPC616BOARD_H
#define VIPC616BOARD_H

#include <vme/vme_api.h>
#include <linux/sched.h>
//-----------------------------------------------------------------------------
// Accessible functions

///
/// \addtogroup drivers Device drivers
///
/// @{

/// @name VIPC 616 carrierboard
//@{ 

/// Returns the irq of the VMEBus bridge
int VIPC616_GetIRQ(void);

/// Returns the handle of the VMEBus bridge
vme_bus_handle_t VIPC616_GetBusNr(void);

///
/// Return the pointer to the base address of the requested slot
///
/// Return 0 if the board or slot doesn't exist
/// @param boardNr = 1, 2, ...
/// @param slotNr  = 1, 2, ...
unsigned char* VIPC616_getCardBaseAddress(int boardNr, int slotNr);

///
/// Checks the ID of the module card
///
/// Return 0 if no errors occur, otherwise which is the first character that
/// is wrong
/// @param boardAddr contains the boardAddrs, which can be acquired 
///        by calling VIPC616_getCardBaseAddress
/// @param IDProm points to an array of characters as it should be
/// @param IDProm size is the length of the array
/// @param swapendian is true if SWAPENDIAN is true for the board on which 
///     the ID PROM is located  
int VIPC616_IP_Module_check (unsigned char* boardAddr, unsigned char* IDProm,
                             unsigned int IDProm_size, int swapendian);
//@}

/// @}

#endif // VIPC616BOARD_H
