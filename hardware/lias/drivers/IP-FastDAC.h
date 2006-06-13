// ============================================================================
//
// = KERNEL_MODULE
//    IP-FastDAC.o
//
// = FILENAME
//    IP-FastDAC.h
//
// = DESCRIPTION
//    Implementation of calls to the DACcard present in LiAS (IP-FastDAC)
//
// = AUTHOR
//    Rene Waarsing <Rene.Waarsing@mech.kuleuven.ac.be>
//
// = CREDITS
//    Based on work from Johan Loeckx (vark)
//
// = COMMENTS
//    For more info, see manual IP-FastDAC of Wavetron microsystems
//
// ============================================================================
//
// $Id $
// $Log: IP-FastDAC.h,v $
// Revision 1.1.2.3  2004/06/28 15:34:43  rwaarsin
// Added docs
//
// Revision 1.1.2.2  2002/11/25 15:44:16  rwaarsin
// Working version 1.0
//
// Revision 1.1.2.1  2002/07/19 14:42:08  rwaarsin
// Initial release
//
//
// ============================================================================
#ifndef IP_FASTDAC_H
#define IP_FASTDAC_H

//----------------------------------------------------------------------------- 
// Definitions of some parametervalues

/// \addtogroup drivers Device drivers
///
/// @{

/// Functions to control the IP-FastDAC card.
/// @name IP-FastDAC
//@{ 

/// Values defing the amplification (cfr. page 2-5) 
#define RANGE_5_V    1
///
///
/// Values defing the amplification (cfr. page 2-5) 
#define RANGE_10_V   0

/// \def GROUP_B 
/// Values of the groups

#define GROUP_A 1
#define GROUP_B 2
#define GROUP_C 3
#define GROUP_D 4


//----------------------------------------------------------------------------- 
// The functions that are availble outside this module
void IP_FastDAC_enable_dac (void);
void IP_FastDAC_disable_dac (void);
void IP_FastDAC_clear_control_register (void);
void IP_FastDAC_clear_strobe_register (void);

// Enable/disbale only works with internal strobing (Strobing on the IP bus)
int  IP_FastDAC_enable_group(char group);
int  IP_FastDAC_disable_group(char group);
void IP_FastDAC_enable_internalStrobing(void);
void IP_FastDAC_disable_internalStrobing(void);

int  IP_FastDAC_set_gain_of_group(char group, char gain);
int  IP_FastDAC_strobe_group (char group);

// Write to a channel. Since the dac work at a conversion rate of 5us, this call waits
// for the time to pass. This will avoid problems of writing too fast to the card.
// Warning: this waiting can cause problems when you call the function in InitModule,
// or from an interrupt or something...
//int  IP_FastDAC_write_float_to_channel (float value, char channel);
int  IP_FastDAC_write_to_channel (unsigned short value, char channel);

int  IP_FastDAC_write_register(unsigned short value, unsigned int regOffset);

//@}
//@}

#endif // end FASTDAC_VME_H
