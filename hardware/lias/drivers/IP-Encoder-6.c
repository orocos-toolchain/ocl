// ============================================================================
//
// = KERNEL_MODULE
//    IP-Encoder-6.o
//
// = FILENAME
//    IP-Encoder-6.c
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
//    It seems not to be possible to really use the overflow registers for 
//    virtual counters, without carefully checking the consistency of the total
//    counter, since the synchronisation between the overflow register, the
//    clearance of this register and the real channel is not possible: E.g. a
//    channel overflows AFTER the overflow register is read, BEFORE it is 
//    cleared or before the channel is read. In both cases problems arise, in
//    the first scenario the encoder data is corrupted for 'always', in the
//    second scenario the encoder data is corrupted for one turn. It is
//    therefore important that these features are only use with great care, or
//    not at all.
//
// ============================================================================
//
// $Id: IP-Encoder-6.c,v 1.1.2.3 2003/08/18 16:05:01 rwaarsin Exp $
// $Log: IP-Encoder-6.c,v $
// Revision 1.1.2.3  2003/08/18 16:05:01  rwaarsin
// Switched from rtlinx (rtl_printf) to kernel (printk)
//
//
// ============================================================================

#define DEBUG 0

//-----------------------------------------------------------------------------
// includes
#include <linux/init.h>
#include <linux/module.h>

#include <asm/io.h>  // for writeb/w/l, readb/w/l

//#include <rtl.h>

#include "VIPC616board.h"
#include "IP-Encoder-6.h"
#include "endianconversion.h"

MODULE_LICENSE("GPL");

//-----------------------------------------------------------------------------
// Local variables
unsigned char* IP_Encoder_6_baseaddress = 0;

//-----------------------------------------------------------------------------
// Local defines
#define MODULE_NAME "IP_Encoder_6"
#define SWAPENDIAN 0

// The encoder card is on slot 1a
#define MYBOARD 1
#define MYSLOT  2

// VMEbus Register map register offsets as described in user manual

// 1. Read-only regs
#define REG_COUNTER_CHANNELS         0x00
#define REG_RESET_ALL_CHANNELS       0x0C
#define REG_OVERFLOW                 0x10
#define REG_INDEX                    0x12
#define REG_UPDOWN                   0x14

// 2. Write-only regs
#define REG_CLEAR_OVERFLOW           0x10
#define REG_CLEAR_INDEX              0x12
#define REG_WRITE_IRQ_VECTOR         0x16
#define REG_WRITE_IRQ_OVERFLOW_MASK  0x18
#define REG_WRITE_I_INDEX_MASK       0x1A

#define IDPROM_SIZE 12
unsigned char IDProm[IDPROM_SIZE] = 
  {0x49, 0x50, 0x41, 0x43, 0xf0, 0xaa, 0xb1, 0x00, 0x00, 0x00, 0x0c, 0x1c};  

#define IMASK_ENABLE_ALL_CHANNELS   0x00

//----------------------------------------------------------------------------- 
// The symbols that are availble outside this module
EXPORT_SYMBOL (IP_Encoder_6_reset_all_channels);
EXPORT_SYMBOL (IP_Encoder_6_clear_overflow);
EXPORT_SYMBOL (IP_Encoder_6_clear_index);
EXPORT_SYMBOL (IP_Encoder_6_get_overflow_register);
EXPORT_SYMBOL (IP_Encoder_6_get_index_register);
EXPORT_SYMBOL (IP_Encoder_6_get_updown_register);
EXPORT_SYMBOL (IP_Encoder_6_get_counter_channel);

//-----------------------------------------------------------------------------
// Function defines
// To read
unsigned short readShort(unsigned int offset)
{
#if SWAPENDIAN
  return swapEndian ( readw (IP_Encoder_6_baseaddress + offset) ); 
#else 
  return readw (IP_Encoder_6_baseaddress + offset); 
#endif
}

unsigned short IP_Encoder_6_get_overflow_register( void )
// Returns the value of the overflow register
{
  return readShort(REG_OVERFLOW);
}

unsigned short IP_Encoder_6_get_index_register( void )
// Returns the value of the index register
{
  return readShort(REG_INDEX);
}

unsigned short IP_Encoder_6_get_updown_register( void )
// Returns the value of the up/down register
{
  return readShort(REG_UPDOWN);
}

unsigned short IP_Encoder_6_get_counter_channel (unsigned int registerNo)
//  Function:	Read a register (cfr. register map)
//
//    registerNo
//      number for a counter channel to easily read a parameter on a 
//      certain channel. If registerNo is outsoide the range, it is adjusted
//      to 1 or 6
{
  // Parameters check
  if (registerNo < 1) registerNo=1;
  if (registerNo > 6) registerNo=6;

  return readShort (0x2 * (registerNo-1));
}

//------------
// To write...

void writeShort(unsigned short flag, unsigned int offset)
// Write a short to a offset on the card
{
#if SWAPENDIAN
    writew (swapEndian(flag), IP_Encoder_6_baseaddress + offset);
#else 
    writew (flag, IP_Encoder_6_baseaddress + offset);
#endif
};

void IP_Encoder_6_reset_all_channels ( void )
// Reads for the 'Reset all channels offset, to reset all channels
{
  readw(IP_Encoder_6_baseaddress + REG_RESET_ALL_CHANNELS);
};

void IP_Encoder_6_clear_overflow ( void )
//  function:	Clear the overflow register
{
  writeShort(0, REG_CLEAR_OVERFLOW);
}

void IP_Encoder_6_clear_index ( void )
//  function:	Clear the indexregister
{
  writeShort(0, REG_CLEAR_INDEX);
}

void IP_Encoder_6_write_IRQ_vector (unsigned short vecNr)
// Set the interrupt vector of encoder
{
  writeShort(vecNr, REG_WRITE_IRQ_VECTOR);
}

void IP_Encoder_6_write_IRQ_overflow_mask (char flag)
// Write to the IRQ overflow mask
{
  writeShort(flag, REG_WRITE_IRQ_OVERFLOW_MASK);
}

void IP_Encoder_6_write_IRQ_index_mask (unsigned char flag)
{
  writeShort(flag, REG_WRITE_I_INDEX_MASK);
}

int IP_Encoder_6_init (void)
// Function  : Initialise the IP-Encoder-6 driver
{
#if DEBUG
  printk("IP_Encoder_6_init entered\n");
#endif
  
  // 1. Get the board base address
  IP_Encoder_6_baseaddress = VIPC616_getCardBaseAddress(MYBOARD, MYSLOT);
  if (!IP_Encoder_6_baseaddress)
  {
    printk ("Error: Can't find baseadress for the IP_Encoder_6\n");
    return -1;
  }

  // 2. Check if the module is there
  if(VIPC616_IP_Module_check(IP_Encoder_6_baseaddress, IDProm, IDPROM_SIZE,
                              SWAPENDIAN ) != 0)
  {
    printk("\n-------------------------------------------------\n");
    printk("Module at position %d%c isn't a IP-Encoder 6\n",
                 MYBOARD, MYSLOT+'a'-1);
    printk("Module IP-Encoder 6 would not be loaded normally!!!\n"); 
    printk("-------------------------------------------------\n\n");
    return -1;
  }

  // 3. Initialise by resetting stuff
  IP_Encoder_6_reset_all_channels ( );
  IP_Encoder_6_clear_overflow ( );
  IP_Encoder_6_clear_index ( );

/*
// We have revision A, so no Interrupts for the encoders
  IP_Encoder_6_write_IRQ_index_mask (IMASK_ENABLE_ALL_CHANNELS);
  IP_Encoder_6_write_IRQ_overflow_mask (IMASK_ENABLE_ALL_CHANNELS);
  IP_Encoder_6_write_IRQ_vector(0x00);
*/
/*
#if DEBUG
  for (i=1;i<7;i++)
  {
    res=encoder_read_register 
      (REG_COUNTER_CHANNELS, i, ENCODER_CARRIER, ENCODER_OFFSET);
    rtl_printf ("Read test (%d): %08X \n",i,res);
  }
#endif*/
  
  // 4. Print a message
  printk(MODULE_NAME " successfully loaded\n");
  return 0;
}

void IP_Encoder_6_cleanup (void)
// Function  : dummy
{
  printk(MODULE_NAME " module unloaded\n");
}

module_init(IP_Encoder_6_init);
module_exit(IP_Encoder_6_cleanup);
