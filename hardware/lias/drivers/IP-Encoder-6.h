// ============================================================================
//
// = KERNEL_MODULE
//    IP-Encoder-6.o
//
// = FILENAME
//    IP-Encoder-6.h
//
// = DESCRIPTION
//    Implementation of calls to the encodercard present in LiAS (IP-Encoder-6)
//
// = AUTHOR
//    Rene Waarsing <Rene.Waarsing@mech.kuleuven.ac.be>
//
// = CREDITS
//    Based on work from Johan Loeckx (vark)
//
// = COMMENTS
//    For more info, see manual IP-Encoder-6 of SBS
//
// ============================================================================
//
// $Id $
// $Log $
//
// ============================================================================

#ifndef IP_ENCODER_6_H
#define IP_ENCODER_6_H

/// \addtogroup drivers Device drivers
///
/// @{

/// Functions to address the IP-Encoder 6 card.
/// @name IP-Encoder 6
//@{ 


//-----------------------------------------------------------------------------
// Accessible functions

/// Reads for the 'Reset all channels offset, to reset all channels
void IP_Encoder_6_reset_all_channels ( void );

///  function:	Clear the overflow register
void IP_Encoder_6_clear_overflow ( void );

///  function:	Clear the indexregister
void IP_Encoder_6_clear_index ( void );

/// Returns the value of the overflow register
unsigned short IP_Encoder_6_get_overflow_register( void );

/// Returns the value of the index register
unsigned short IP_Encoder_6_get_index_register( void );

/// Returns the value of the up/down register
unsigned short IP_Encoder_6_get_updown_register( void );

///  Function:	Read a counter_channel (1..6)
unsigned short IP_Encoder_6_get_counter_channel (unsigned int registerNo);

///@}
///@}
#endif // IP_ENCODER_6_H
