// ============================================================================
//
// = KERNEL_MODULE
//    IP-Digital_24.o
//
// = FILENAME
//    IP-Digital_24.c
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
// $Id: IP-Digital_24.c,v 1.1.2.2 2003/08/18 15:58:40 rwaarsin Exp $
// $Log: IP-Digital_24.c,v $
// Revision 1.1.2.2  2003/08/18 15:58:40  rwaarsin
// Switched from rtlinx (rtl_printf) to kernel (printkt
//
// Revision 1.1.2.1  2002/07/19 14:42:08  rwaarsin
// Initial release
//
//
// ============================================================================

#define DEBUG 0

//-----------------------------------------------------------------------------
// includes
#include <linux/init.h>
#include <linux/module.h>

#include <asm/io.h>  // for writeb/w/l, readb/w/l

#include "VIPC616board.h"
#include "IP-Digital_24.h"
#include "endianconversion.h"

MODULE_LICENSE("GPL");

//----------------------------------------------------------------------------- 
// The symbols that are availble outside this module
EXPORT_SYMBOL_NOVERS (IP_Digital_24_set_bit_of_channel);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_clear_bit_of_channel);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_get_bit_of_channel);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_readback_line_1_16);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_readback_line_17_24);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_directread_line_1_16);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_directread_line_17_24);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_output_line_1_16);
EXPORT_SYMBOL_NOVERS (IP_Digital_24_output_line_17_24);

//-----------------------------------------------------------------------------
// Local variables
unsigned char* IP_Digital_24_baseaddress = 0;

//-----------------------------------------------------------------------------
// Local defines
#define MODULE_NAME "IP_Digital_24"
#define REG_READBACK_1_16      0x00  // read from latch
#define REG_READBACK_17_24     0x02  // read from latch
#define REG_DIRECTREAD_1_16    0x04  // read the state of the IO line
#define REG_DIRECTREAD_17_24   0x06  // read the state of the IO line
#define REG_OUTPUT_1_16        0x00  // write to latch
#define REG_OUTPUT_17_24       0x02  // write to latch

// The encoder card is on slot 2a
#define MYBOARD 2
#define MYSLOT  1

#define SWAPENDIAN  0

#define IDPROM_SIZE 12
unsigned char IDProm[IDPROM_SIZE] = 
  {0x49, 0x50, 0x41, 0x43, 0xf0, 0x11, 0xb2, 0x00, 0x00, 0x00, 0x0c, 0xa7};  

//-----------------------------------------------------------------------------
// Local function defintions and implementations
unsigned short IP_Digital_24_readreg (unsigned int registerOffset)
{
#if SWAPENDIAN
  return swapEndian( readw (IP_Digital_24_baseaddress + registerOffset));
#else
  return readw (IP_Digital_24_baseaddress + registerOffset);
#endif
}

void IP_Digital_24_writereg (unsigned short value, unsigned int registerOffset)
{
#if SWAPENDIAN
    writew (swapEndian(value), IP_Digital_24_baseaddress + registerOffset);
#else 
    writew (value, IP_Digital_24_baseaddress + registerOffset);
#endif
}

//-----------------------------------------------------------------------------
// The functions that are available outside this module

int IP_Digital_24_set_bit_of_channel (unsigned int outputline)
// Set a bit of a channel nr
// Returns -1 on error
{
  unsigned short temp;
  unsigned int position;

  // 1. check range
  if ( (outputline < 1) | (outputline > 24) ) return -1;
 
  position = outputline - 1;
  // line 1 eq bit 0

  if (position <= 15)
  { // bit 0-15 or line 1-16
    temp = IP_Digital_24_directread_line_1_16();
    temp |= (1<<position);	
    IP_Digital_24_output_line_1_16 (temp);
  }
  else
  {	// bit 16-23 or line 17-
    temp = IP_Digital_24_directread_line_17_24();
    temp |= (1<< (position-16));	
    IP_Digital_24_output_line_17_24 (temp);
  }
  //printk("IP_Digital_24_set_bit_of_channel\n");
  return 0;
}

int IP_Digital_24_clear_bit_of_channel (unsigned int outputline)
// Clear a bit of a channel nr
// Returns -1 on error
{
  unsigned short temp;
  unsigned int position;

  // 1. check range
  if ( (outputline < 1) | (outputline > 24) ) return -1;
 
  position = outputline - 1;
  // line 1 eq bit 0
  
  if (position <= 15)
  {  // bit 0-15 or line 1-16
    temp = IP_Digital_24_directread_line_1_16();
    temp &= ~(1<<position);	
    IP_Digital_24_output_line_1_16 (temp);
  }
  else
  {	// bit 16-23 or line 17-
    temp = IP_Digital_24_directread_line_17_24();
    temp &= ~(1 << (position-16));	
    IP_Digital_24_output_line_17_24 (temp);
  }
  return 0;
}

char IP_Digital_24_get_bit_of_channel (unsigned int inputline)
// Get a bit of a line
// Returns -1 on error
{
  unsigned short temp;
  unsigned int position;

  // 1. check range
  if ( (inputline < 1) | (inputline > 24) ) return -1;
 
  position = inputline - 1;
  // line 1 eq bit 0

  if (position <= 15)
  { // bit 0-15 or line 1-16
    temp = IP_Digital_24_directread_line_1_16();
    temp &= (1<<position);
  }
  else
  {	// bit 16-23 or line 17-
    temp = IP_Digital_24_directread_line_17_24();
    temp &= (1<< (position-16));
  }

  if (temp == 0) return 0;
  else return 1;
}

unsigned short IP_Digital_24_readback_line_1_16 (void)
// Read the status of line 1-16 (what has been written from the card)
{
  return IP_Digital_24_readreg (REG_READBACK_1_16);
}

unsigned short IP_Digital_24_readback_line_17_24 (void)
// Read the status of line 17-24 (what has been written from the card)
{
  return IP_Digital_24_readreg (REG_READBACK_17_24);
}

unsigned short IP_Digital_24_directread_line_1_16 (void)
// Read the value of line 1-16 (what is really on the line) 
{
   return IP_Digital_24_readreg (REG_DIRECTREAD_1_16);
}

unsigned short IP_Digital_24_directread_line_17_24 (void)
// Read the value of line 1-16 (what is really on the line) 
{
    return IP_Digital_24_readreg (REG_DIRECTREAD_17_24);
}

void IP_Digital_24_output_line_1_16 (unsigned short value)
// Write something to the lines 1-16
{
    IP_Digital_24_writereg (value, REG_OUTPUT_1_16);
}

void IP_Digital_24_output_line_17_24 (unsigned short value)
// Write something to the lines 17-24
{
    IP_Digital_24_writereg (value, REG_OUTPUT_17_24);
}

int IP_Digital_24_init(void)
// Initialisation of the IP card : no initialisation
{
#if DEBUG
  printk("IP_Digital_24_init entered\n");
#endif
  // 1. Get the Base Address (global var)
  IP_Digital_24_baseaddress = VIPC616_getCardBaseAddress(MYBOARD, MYSLOT);

  // 2. Check if the module is there
  if(VIPC616_IP_Module_check(IP_Digital_24_baseaddress, IDProm, IDPROM_SIZE,
                            SWAPENDIAN) != 0)
  {
    printk("\n-------------------------------------------------\n");
    printk("Module at position %d%c, it isn't a IP_Digital_24\n",
                 MYBOARD, MYSLOT+'a'-1); 
    printk("Module IP_Digital_24 will not be loaded!!!\n"); 
    printk("-------------------------------------------------\n\n");
    return 1;
  }

  printk(MODULE_NAME " successfully loaded\n");
  return 0;
}

void IP_Digital_24_cleanup(void)
{
  printk(MODULE_NAME " module unloaded\n");
}

module_init (IP_Digital_24_init);
module_exit (IP_Digital_24_cleanup);
