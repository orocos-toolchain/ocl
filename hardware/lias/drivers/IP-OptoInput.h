// ============================================================================
//
// = KERNEL_MODULE
//    IP-OptoInput.o
//
// = FILENAME
//    IP-OptoInput.h
//
// = DESCRIPTION
//    Implementation of calls to the OptoInput card present in LiAS
//
// = AUTHOR
//    Rene Waarsing <Rene.Waarsing@mech.kuleuven.ac.be>
//    Johan Rutgeerts <Johan.Rutgeerts@student.kuleuven.ac.be>
//    Wim Meeussen <Wim.Meeussen@student.kuleuven.ac.be>
//
// = CREDITS
//    Based on work from Johan Loeckx (vark)
//
// = COMMENTS
//    For more info, see manual IP-OptoInput of SBS
//
// ============================================================================
//
// $Id: IP-OptoInput.h,v 1.1.2.2 2004/06/28 15:34:43 rwaarsin Exp $
// $Log: IP-OptoInput.h,v $
// Revision 1.1.2.2  2004/06/28 15:34:43  rwaarsin
// Added docs
//
// Revision 1.1.2.1  2002/07/19 14:42:08  rwaarsin
// Initial release
//
//
//
// ============================================================================

#ifndef IP_OPTOINPUT_H
#define IP_OPTOINPUT_H


/// \addtogroup drivers Device drivers
///
/// @{

/// Functions to address the IP-OptoInput card.
/// @name IP-OptoInput
//@{ 


//-----------------------------------------------------------------------------
// Accessible functions

/// Reads the Input Data Register (all inputs)
unsigned short IP_OptoInput_Read_IDR( void );

/// Switches the Global Interrupt on or off
///
/// @param boolean true ==> set Global Interrupt Enable
/// @param boolean false ==> reset Global Interrupt Enable
void IP_OptoInput_Set_GIE(int boolean);

/// Reads the Global Interrupt Request Flag (retuns 1 or 0)
int IP_OptoInput_Read_GIRF( void );

// -----------------------
// Enabling interrupts.
/// Write unsigned short to Interrupt Enable Register Rising Edge
void IP_OptoInput_Write_IERRE(unsigned short writeIERRE);

/// Read Interrupt Enable Register Rising Edge
unsigned short IP_OptoInput_Read_IERRE( void );

/// Set some bits of Interrupt Enable Register Rising Edge.
///
/// E.g. setIERRE = 0x010A ==> interrupt for channel 2, 4 and 9 are set
///                            other channels are not changed 
void IP_OptoInput_Set_IERRE(unsigned short setIERRE);

/// Reset some bits of Interrupt Enable Register Rising Edge.
///
/// E.g. resetIERRE = 0x010A ==> interrupt for channel 2, 4 and 9 are reset
///                              other channels are not changed
void IP_OptoInput_Reset_IERRE(unsigned short resetIERRE);

/// Write unsigned short to Interrupt Enable Register Falling Edge
void IP_OptoInput_Write_IERFE(unsigned short writeIERFE);

/// Read Interrupt Enable Register Falling Edge
unsigned short IP_OptoInput_Read_IERFE( void );

// Set several bits of Interrupt Enable Register Falling Edge.
///
/// E.g. setIERFE = 0x010A ==> interrupt for channel 2, 4 and 9 are set
///                            other channels are not changed 
void IP_OptoInput_Set_IERFE(unsigned short setIERFE);

/// Reset some bits of Interrupt Enable Register Falling Edge.
///
/// E.g. resetIERFE = 0x010A ==> interrupt for channel 2, 4 and 9 are reset
///                              other channels are not changed
void IP_OptoInput_Reset_IERFE(unsigned short resetIERFE);


// -----------------------
// Responding to interrupts.

/// Read Interrupt Status Register Rising Edge.
unsigned short IP_OptoInput_Read_ISRRE( void );

/// Write unsigned short to Interrupt Status Register Rising Edge.
void IP_OptoInput_Write_ISRRE(unsigned short writeISRRE);

/// Reset some bits of Interrupt Status Register Rising Edge.
///
/// E.g. resetISRRE = 0x010A ==> reset status for channel 2, 4 and 9
///                              other channels are not changed
void IP_OptoInput_reset_ISRRE(unsigned short resetISRRE);

/// Read Interrupt Status Register Falling Edge.
unsigned short IP_OptoInput_Read_ISRFE( void );

/// Write unsigned short to Interrupt Status Register Falling Edge.
void IP_OptoInput_Write_ISRFE(unsigned short writeISRFE);

/// Reset some bits of Interrupt Status Register Falling Edge.
///
/// E.g. resetISRRE = 0x010A ==> reset status for channel 2, 4 and 9
///                              other channels are not changed
void IP_OptoInput_reset_ISRFE(unsigned short resetISRFE);

/// Sets the vector of the interrupt (just a number?)
void IP_OptoInput_set_Interrupt_Vector_Register(unsigned short vecNr);

/// If there is not yet a routine attached, attach routine and return  the
/// routineId (a number between 0 and maximum_routines, otherwise return -1
int IP_OptoInput_attachInterruptRoutine ( int (*routine) (void));

/// Detach the routine and return 0
int IP_OptoInput_detachInterruptRoutine (unsigned int routineId);

//@}
//@}
#endif // IP_OPTOINPUT_H
