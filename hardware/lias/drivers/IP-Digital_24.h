// ============================================================================
//
// = KERNEL_MODULE
//    IP-Digital_24.o
//
// = FILENAME
//    IP-Digital_24.h
//
// = DESCRIPTION
//    Implementation of calls to the Digital IOcard present in LiAS
//
// = AUTHOR
//    Rene Waarsing <Rene.Waarsing@mech.kuleuven.ac.be>
//
// = CREDITS
//    Based on work from Johan Loeckx (vark)
//
// = COMMENTS
//    For more info, see manual IP-Digital 24 of Green Spring
//
// ============================================================================
//
// $Id $
// $Log: IP-Digital_24.h,v $
// Revision 1.1.2.2  2004/06/28 15:34:43  rwaarsin
// Added docs
//
// Revision 1.1.2.1  2002/07/19 14:42:08  rwaarsin
// Initial release
//
//
// ============================================================================

#ifndef DIGITAL24_VME_H
#define DIGITAL24_VME_H

/// \addtogroup drivers Device drivers
///
/// @{

/// Functions to address the Digital 24 card.
/// @name Digital 24
//@{ 

//----------------------------------------------------------------------------- 
// The symbols that are availble outside this module

/// Bit access
int IP_Digital_24_set_bit_of_channel   (unsigned int outputline);
/// Bit access
int IP_Digital_24_clear_bit_of_channel (unsigned int outputline);
/// Bit access
char IP_Digital_24_get_bit_of_channel  (unsigned int inputline);

/// Direct Access 'read'
unsigned short IP_Digital_24_readback_line_1_16 (void);
/// Direct Access 'read'
unsigned short IP_Digital_24_readback_line_17_24 (void);
/// Direct Access 'read'
unsigned short IP_Digital_24_directread_line_1_16 (void);
/// Direct Access 'read'
unsigned short IP_Digital_24_directread_line_17_24 (void);

/// Direct Access 'write'
void IP_Digital_24_output_line_1_16 (unsigned short value);
/// Direct Access 'write'
void IP_Digital_24_output_line_17_24 (unsigned short value);

///@}
///@}
#endif
