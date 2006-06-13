// ============================================================================
//
// = KERNEL_MODULE
//    IP-FastDAC.o
//
// = FILENAME
//    IP-FastDAC.c
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
// $Id: IP-FastDAC.c,v 1.1.2.3 2003/08/18 16:05:29 rwaarsin Exp $
// $Log: IP-FastDAC.c,v $
// Revision 1.1.2.3  2003/08/18 16:05:29  rwaarsin
// Switched from rtlinx (rtl_printf) to kernel (printk)
//
// Revision 1.1.2.2  2002/11/25 15:43:47  rwaarsin
// Working version 1.0
//
//
// ============================================================================

#define DEBUG 0

//-----------------------------------------------------------------------------
// includes
#include <linux/init.h>
#include <linux/module.h>

#include <asm/io.h>  // for writeb/w/l, readb/w/l

#include <linux/delay.h>    // used for nanosleep

//#include <rtl.h>

#include "VIPC616board.h"
#include "IP-FastDAC.h"
#include "endianconversion.h"

MODULE_LICENSE("GPL");

//----------------------------------------------------------------------------- 
// The symbols that are availble outside this module
EXPORT_SYMBOL_NOVERS (IP_FastDAC_enable_dac);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_disable_dac);

EXPORT_SYMBOL_NOVERS (IP_FastDAC_clear_control_register);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_clear_strobe_register);

EXPORT_SYMBOL_NOVERS (IP_FastDAC_enable_group);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_disable_group);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_enable_internalStrobing);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_disable_internalStrobing);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_set_gain_of_group);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_strobe_group);

//EXPORT_SYMBOL_NOVERS (IP_FastDAC_write_float_to_channel);
EXPORT_SYMBOL_NOVERS (IP_FastDAC_write_to_channel);

EXPORT_SYMBOL_NOVERS (IP_FastDAC_write_register);

//-----------------------------------------------------------------------------
// Local variables
unsigned char* IP_FastDAC_baseaddress = 0;

// The strobe register and the control register are read only. To control only
// one bit, we have to keep a copy of the written register value
unsigned short FastDAC_Control_Register; 

//-----------------------------------------------------------------------------
// Local defines
#define MODULE_NAME "IP_FastDAC"
#define SWAPENDIAN		0

// The encoder card is on slot 2c
#define MYBOARD 2
#define MYSLOT  3

// FastDAC registers cfr. page 2-3
#define CHANNEL_REGISTER_OFFSET  0x00    // Start location of the channels
#define CONTROL_REGISTER         0x20    // Control register
#define STROBE_REGISTER          0x22    // Strobe register

// Control Register 'Commands'  cfr. page 2-5
// (These are bit positions)
#define CONTROL_REG_DAC         0
#define CONTROL_REG_IPSTROBING  1
#define CONTROL_REG_GRP_A_GAIN  4
#define CONTROL_REG_GRP_B_GAIN  5
#define CONTROL_REG_GRP_C_GAIN  6
#define CONTROL_REG_GRP_D_GAIN  7
#define CONTROL_REG_GROUP_A     8
#define CONTROL_REG_GROUP_B     9
#define CONTROL_REG_GROUP_C    10
#define CONTROL_REG_GROUP_D    11

//-----------------------------------------------------------------------------
// Local function defintions and implementations

#if 0
short IP_FastDAC_convert_float_to_13bit (float value, char range)
// Convert a float to a 13bit value, using the range of the channel
// range = {RANGE_10_V, RANGE_5_V) 
// if float is outside the range, it is adjusted to the range
{
  float maximumRange = 10;
  if (range == RANGE_5_V) maximumRange = 5;

  // 1. Check the range
  if (value < -maximumRange) value = -maximumRange; 
  if (value >  maximumRange) value =  maximumRange; 

  // 2. Scale the value;
  value /= maximumRange;

  // 3. Return the short 13bit value (cfr. 2-4 & 2-8)  
  return ((short) ((value * 4095.0) + 4096));
}
#endif

//-----------------------------------------------------------------------------
// The functions that are available outside this module

void IP_FastDAC_enable_dac (void)
// Enable the dac by writing a 1 to bit CONTROL_REG_DAC
{
  unsigned short command;

  command = 1 << CONTROL_REG_DAC;

  // Following line sets the bitnumber 'command' to value
  FastDAC_Control_Register |= command;
#if DEBUG
  printk("Enable dac: control register: %x\n", FastDAC_Control_Register);
#endif
  // Write it to the FastDAC
  IP_FastDAC_write_register (FastDAC_Control_Register, CONTROL_REGISTER);
}

void IP_FastDAC_disable_dac (void)
// Disable the dac by writing a 1 to bit CONTROL_REG_DAC
{
  unsigned short command;

  command = 1 << CONTROL_REG_DAC;

  // Following line clears the bitnumber 'command' to value
  FastDAC_Control_Register &= ~command;

  // Write it to the FastDAC
  IP_FastDAC_write_register (FastDAC_Control_Register, CONTROL_REGISTER);
}

void IP_FastDAC_clear_control_register (void)
{
  // Reset to 0 and output
  FastDAC_Control_Register = 0x0000;
  IP_FastDAC_write_register(FastDAC_Control_Register, CONTROL_REGISTER);
}

void IP_FastDAC_clear_strobe_register (void)
{
  // Reset to 0 and output
  IP_FastDAC_write_register(0x0000, STROBE_REGISTER);
}

int IP_FastDAC_write_to_channel (unsigned short value, char channel)
// Function return 0 on success, -1 on failure
// Channel should be between 1 and 8
{
  //printk("IP_FastDAC_write_register: %i, %i\n", value, (unsigned int) channel);
  // 1. Check the range     
  if (channel > 8) return -1; 
  if (channel < 1) return -1;

  // 2. Write to the data register
  IP_FastDAC_write_register
    (value, CHANNEL_REGISTER_OFFSET + (0x02 * (channel - 1)));

  //printk("IP_FastDAC_write_register: %i\n", value);

  // 3. Strobe
  switch (channel)
  {
  case 1: case 2:	// Group A
    IP_FastDAC_strobe_group (GROUP_A);
    break;
  case 3: case 4:	// Group B
    IP_FastDAC_strobe_group (GROUP_B);
    break;
  case 5: case 6:	// Group C
    IP_FastDAC_strobe_group (GROUP_C);
    break;
  case 7: case 8:	// Group D
    IP_FastDAC_strobe_group (GROUP_D);
    break;
  }

  //
  // Johan Rutgeerts: I do not want to do this.
  // Just do not write too fast to the card.
#if 0
  // 4. Now we wait for 10 us to solve the problem which might occur if we 
  //    write to the card to fast
  udelay(10);
#endif
  
  return 0;
}

#if 0
int IP_FastDAC_write_float_to_channel (float value, char channel)
// Write a float to the channel. First calculate the right short, then write that
// to the channel. If the float is clipped, i.e. if outside the range, adjusted.
// Returns -1 on error, 0 on success
// Channel should be between 1 and 8
{
  unsigned short amplification, mask;

  // 1. Get the amplifictaion of the group
  switch ( channel)
  {
  case 1: case 2:
    mask = 1 << CONTROL_REG_GRP_A_GAIN;
    break;
  case 3: case 4:
    mask = 1 << CONTROL_REG_GRP_B_GAIN;
    break;
  case 5: case 6:
    mask = 1 << CONTROL_REG_GRP_C_GAIN;
    break;
  case 7: case 8:
    mask = 1 << CONTROL_REG_GRP_D_GAIN;
    break;
  default:
    // If the default is called, something was wrong with the channelnr
    return -1;
  }
  amplification = FastDAC_Control_Register & mask;
  // (RANGE_10_V = 0. If a bit is set, it is RANGE_5_V)
  if (amplification != RANGE_10_V) amplification = RANGE_5_V;
  
  // 1. Get the amplifictaion of the group
  return IP_FastDAC_write_to_channel(
    IP_FastDAC_convert_float_to_13bit (value, amplification), channel);
}
#endif

int  IP_FastDAC_write_register(unsigned short value, unsigned int regOffset)
// Write value to the registerOffset
// Check if the offset is ok.
{
#if DEBUG
  printk ("IP_FastDAC: Writing 0x%x to register 0x%x\n", value, regOffset);
//  printk ("IP_FastDAC: Control register = 0x%x\n",FastDAC_Control_Register);
#endif
  // 1. Check the register offset
  if (regOffset & 1)    return -1; // Offset is odd...
  if (regOffset > 0x22) return -1; // Offset out off bounds

  // 2. Write value to registerOffset
#if SWAPENDIAN
    writew (swapEndian(value), IP_FastDAC_baseaddress + regOffset);
#else 
    writew (value, IP_FastDAC_baseaddress + regOffset);
#endif
  return 0;
}

int  IP_FastDAC_enable_group(char group)
// Returns -1 on failure (group out of bounds)
// Returns 0 on success
// Enables group (1...4)
{
  unsigned short mask = 0;

  // 1. Check the group
  if (group < 1) return -1;
  if (group > 4) return -1;

  // 2. Enable the group
  switch ( group )
  {
  case GROUP_A:
    mask = 1 << CONTROL_REG_GROUP_A;
    break;
  case GROUP_B:
    mask = 1 << CONTROL_REG_GROUP_B;
    break;
  case GROUP_C:
    mask = 1 << CONTROL_REG_GROUP_C;
    break;
  case GROUP_D:
    mask = 1 << CONTROL_REG_GROUP_D;
    break;
  }

  FastDAC_Control_Register &= ~mask;
  return 
    IP_FastDAC_write_register(FastDAC_Control_Register, CONTROL_REGISTER);
}

int IP_FastDAC_disable_group(char group)
// Returns -1 on failure (group out of bounds)
// Returns 0 on success
// Disables group (1...4)
{
  unsigned short mask = 0;

  // 1. Check the group
  if (group < 1) return -1;
  if (group > 4) return -1;

  // 2. Enable the group
  switch ( group )
  {
  case GROUP_A:
    mask = 1 << CONTROL_REG_GROUP_A;
    break;
  case GROUP_B:
    mask = 1 << CONTROL_REG_GROUP_B;
    break;
  case GROUP_C:
    mask = 1 << CONTROL_REG_GROUP_C;
    break;
  case GROUP_D:
    mask = 1 << CONTROL_REG_GROUP_D;
    break;
  }

  FastDAC_Control_Register |= mask;
  return IP_FastDAC_write_register
          (FastDAC_Control_Register, CONTROL_REGISTER);
}

void IP_FastDAC_enable_internalStrobing(void)
{
  unsigned short command;

  command = 1 << CONTROL_REG_IPSTROBING;

  // Following line sets the bitnumber 'command' to value
  FastDAC_Control_Register |= command;

  // Write it to the FastDAC
  IP_FastDAC_write_register (FastDAC_Control_Register, CONTROL_REGISTER);
}

void IP_FastDAC_disable_internalStrobing(void)
{
  unsigned short command;

  command = 1 << CONTROL_REG_IPSTROBING;

  // Following line clears the bitnumber 'command' to value
  FastDAC_Control_Register &= ~command;

  // Write it to the FastDAC
  IP_FastDAC_write_register (FastDAC_Control_Register, CONTROL_REGISTER);
}


int IP_FastDAC_set_gain_of_group(char group, char gain)
// Returns -1 on failure (group out of bounds)
// Returns 0 on success
// Sets range of group (1...4) to 5 (gain = RANGE_5_V) 
// or 10 (gain = RANGE_10_V)
{
  unsigned short mask = 0;
  //printk("IP_FastDAC_set_gain_of_group: %i, %i\n", (unsigned int) group, (unsigned int) gain);
  
  // 1. Check the group
  if (group < 1) return -1;
  if (group > 4) return -2;

  // 2. Check the range/gain
  if ((gain != RANGE_5_V) && (gain != RANGE_10_V)) return -3;

  // 3. Set the range of the group
  switch ( group )
  {
  case GROUP_A:
    mask = 1 << CONTROL_REG_GRP_A_GAIN;
    break;
  case GROUP_B:
    mask = 1 << CONTROL_REG_GRP_B_GAIN;
    break;
  case GROUP_C:
    mask = 1 << CONTROL_REG_GRP_C_GAIN;
    break;
  case GROUP_D:
    mask = 1 << CONTROL_REG_GRP_D_GAIN;
    break;
  }
  if (gain == RANGE_5_V)
    FastDAC_Control_Register |= mask;
  else
    FastDAC_Control_Register &= (~mask);

  return IP_FastDAC_write_register
           (FastDAC_Control_Register, CONTROL_REGISTER);
}

int IP_FastDAC_strobe_group (char group)
// Returns -1 on failure (group out of bounds)
// Returns 0 on success
// Strobe one group (1..4)
{
  short strobeValue = 0;

  // 1. Check the group
  if (group < 1) return -1;
  if (group > 4) return -1;

  // 2. Strobe group
  strobeValue = 1 << (group -1);
  // Write it to the FastDAC
  return IP_FastDAC_write_register(strobeValue, STROBE_REGISTER);
}

int IP_FastDAC_init(void)
// Initialisation of IP card
{
#if DEBUG
  printk("IP_FastDAC_init entered\n");
#endif
  
  // 1. Get the Base Address
  IP_FastDAC_baseaddress = VIPC616_getCardBaseAddress(MYBOARD, MYSLOT);

  // 2. Initialise IP-FastDAC as if using IP Reset
  //
  // IP Reset is done on the board, but we can't do this in software.
  // Therefore, we reset al the registers manually
  
  // After reset, the Control reg and the strobe reg have value 0x00
  // (cfr. manual page 2-5)
  IP_FastDAC_clear_strobe_register();
  IP_FastDAC_clear_control_register(); 
  // Clear controlregister means (cfg. p2-5 of the manual):
  // CR0    -> clear DACs output
  // CR1    -> Disable Strobe via Strobe line on IP board
  // CR4-7  -> Amplification to 10 V
  // CR8/11 -> Enable groups A till D

  // 3. Print a message
  printk(MODULE_NAME " successfully loaded\n");

  return 0;
}

void IP_FastDAC_cleanup(void)
// clean ip of module: reset
{
  // 1. Reset IP-FastDAC
  IP_FastDAC_clear_strobe_register();
  IP_FastDAC_clear_control_register();


  printk(MODULE_NAME " module unloaded\n");
}

module_init (IP_FastDAC_init);
module_exit (IP_FastDAC_cleanup);
