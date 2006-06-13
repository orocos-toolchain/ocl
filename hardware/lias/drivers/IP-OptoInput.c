// ============================================================================
//
// = KERNEL_MODULE
//    IP-OptoInput.o
//
// = FILENAME
//    IP-OptoInput.cpp
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
// $Id: IP-OptoInput.c,v 1.1.2.2 2003/08/18 16:06:17 rwaarsin Exp $
// $Log: IP-OptoInput.c,v $
// Revision 1.1.2.2  2003/08/18 16:06:17  rwaarsin
// Changed to new version of kernel, thus a new version of the Universe module
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
#include <linux/fs.h>
#include <linux/interrupt.h>

#include <asm/io.h>  // for writeb/w/l, readb/w/l

#include "VIPC616board.h"
#include "IP-OptoInput.h"
#include "endianconversion.h"

#include <vme/vme.h>
MODULE_LICENSE("GPL");

//-----------------------------------------------------------------------------
// Local defines
#define MODULE_NAME "IP_OptoInput"

#define SWAPENDIAN 0

// The optoinput card is on slot 2b
#define MYBOARD 2
#define MYSLOT  2

// VMEbus Register map register offsets as described in user manual
#define REG_DATAREG       0x00
#define REG_INTCONT       0x03
#define REG_INTENALH      0x04
#define REG_INTENAHL      0x06
#define REG_INTSTATLH     0x08
#define REG_INTSTATHL     0x0A
#define REG_INTVEC        0x0D
#define REG_DEBTIME       0x0F

// An interrupt vector.
#define IP_OPTOINPUT_INTERRUPT_VECTOR 0x34
#define IP_OPTOINPUT_IRQ_LEVEL        0x03

#define MAXIMUM_EXTERNAL_INTERRUPT_ROUTINES 16

//-----------------------------------------------------------------------------
// Local variables
unsigned char* IP_OptoInput_baseaddress = 0;
vme_interrupt_handle_t interruptHandle;

int myInterruptId = -1;
typedef int (* ExternalInterruptRoutineTp)(void);
ExternalInterruptRoutineTp 
  externalInterruptRoutine[MAXIMUM_EXTERNAL_INTERRUPT_ROUTINES];

// Not really used, only in the interrupt request function, to provide a unique
// ID
static struct file_operations file_ops = 
{
  open:NULL,
  release:NULL,
  read:NULL,
  write:NULL
};

//----------------------------------------------------------------------------- 
// The symbols that are availble outside this module
EXPORT_SYMBOL (IP_OptoInput_Read_IDR);
EXPORT_SYMBOL (IP_OptoInput_Set_GIE);
EXPORT_SYMBOL (IP_OptoInput_Read_GIRF);
EXPORT_SYMBOL (IP_OptoInput_Write_IERRE);
EXPORT_SYMBOL (IP_OptoInput_Read_IERRE);
EXPORT_SYMBOL (IP_OptoInput_Set_IERRE);
EXPORT_SYMBOL (IP_OptoInput_Reset_IERRE);
EXPORT_SYMBOL (IP_OptoInput_Write_IERFE);
EXPORT_SYMBOL (IP_OptoInput_Read_IERFE);
EXPORT_SYMBOL (IP_OptoInput_Set_IERFE);
EXPORT_SYMBOL (IP_OptoInput_Reset_IERFE);
EXPORT_SYMBOL (IP_OptoInput_Read_ISRRE);
EXPORT_SYMBOL (IP_OptoInput_Write_ISRRE);
EXPORT_SYMBOL (IP_OptoInput_reset_ISRRE);
EXPORT_SYMBOL (IP_OptoInput_Read_ISRFE);
EXPORT_SYMBOL (IP_OptoInput_Write_ISRFE);
EXPORT_SYMBOL (IP_OptoInput_reset_ISRFE);
EXPORT_SYMBOL (IP_OptoInput_set_Interrupt_Vector_Register);
EXPORT_SYMBOL (IP_OptoInput_attachInterruptRoutine);
EXPORT_SYMBOL (IP_OptoInput_detachInterruptRoutine);

//-----------------------------------------------------------------------------
// Local functions
static irqreturn_t IP_OptoInput_InterruptHandler (int irq, void* dev_id, struct pt_regs* pt_regs)
{
  int interruptVector;
  unsigned short risingEdge, fallingEdge;
  unsigned int i;

  // First test if this is our LEvel
  if (vme_interrupt_asserted(VIPC616_GetBusNr(), interruptHandle) )
  {
    // Get the vector... (why??)
    vme_interrupt_vector(VIPC616_GetBusNr(), interruptHandle, &interruptVector);
    if (interruptVector != IP_OPTOINPUT_INTERRUPT_VECTOR) 
    {
      printk(KERN_NOTICE MODULE_NAME ":Interrupt vector 0x%X inequal to the expected 0x%X!?\n",
                interruptVector, IP_OPTOINPUT_INTERRUPT_VECTOR);
    }
    risingEdge  = IP_OptoInput_Read_ISRRE();
    fallingEdge = IP_OptoInput_Read_ISRFE();
#if DEBUG
    printk(KERN_NOTICE MODULE_NAME ":Interrupt called\n");
    printk(KERN_NOTICE MODULE_NAME ":Status register rising edge:  0x%x\n", risingEdge);
    printk(KERN_NOTICE MODULE_NAME ":Status register falling edge: 0x%x\n", fallingEdge);
#endif

    for (i = 0; i < MAXIMUM_EXTERNAL_INTERRUPT_ROUTINES; i++)
      if (externalInterruptRoutine[i]) (externalInterruptRoutine[i])();

    IP_OptoInput_Write_ISRRE(risingEdge);
    IP_OptoInput_Write_ISRFE(fallingEdge);
    // Clear the interrupt
    vme_interrupt_clear(VIPC616_GetBusNr(), interruptHandle);
    return IRQ_HANDLED;
  }
  return IRQ_NONE;
}

int IP_Module_check(void)
{
  #define IDPROM_SIZE 13
  unsigned char IDProm[IDPROM_SIZE] = 
    {0x49, 0x50, 0x41, 0x43, 0xb3, 0x1a, 0x10, 0x00, 0x00, 0x00, 0x0d, 0x1f, 0x0a};  
  int i;

  for (i = 0; i< IDPROM_SIZE; i++)
  {
#if SWAPENDIAN
    if (*(IP_OptoInput_baseaddress + 0x80 + 2*i) != IDProm[i])
#else
    if (*(IP_OptoInput_baseaddress + 0x81 + 2*i) != IDProm[i])
#endif
      return i+1;
  }

  return 0;
}

// Function defines

unsigned short readShort(unsigned int offset)
{
#if SWAPENDIAN
  return swapEndian ( readw (IP_OptoInput_baseaddress + offset) ); 
#else 
  return readw (IP_OptoInput_baseaddress + offset); 
#endif
}

void writeShort(unsigned short flag, unsigned int offset)
// Write a short to a offset on the card
{
#if SWAPENDIAN
    writew (swapEndian(flag), IP_OptoInput_baseaddress + offset);
#else 
    writew (flag, IP_OptoInput_baseaddress + offset);
#endif
}


//-----------------------------------------------------------------------------
// Global functions

int IP_OptoInput_attachInterruptRoutine ( ExternalInterruptRoutineTp routine )
// If there is not yet a routine attached, attach routine and return the 
// routineID, otherwise return -1
{
  int i;
  for (i = 0; i < MAXIMUM_EXTERNAL_INTERRUPT_ROUTINES; i++)
  {
    if (externalInterruptRoutine[i] == 0)
    {
      externalInterruptRoutine[i] = routine;
      return i;
    }
  }
  // Otherwise: error
  return -1;
}

int IP_OptoInput_detachInterruptRoutine (unsigned int routineId)
// If routine was aateched, detach it and return 0, otherwise return -1
{
  externalInterruptRoutine[routineId] = 0;
  return 0;
}

unsigned short IP_OptoInput_Read_IDR( void )
// Reads the Input Data Register (all inputs)
{
  return readShort(REG_DATAREG);
}

void IP_OptoInput_Set_GIE(int boolean)
// boolean = 1 ==> set Global Interrupt Enable
// boolean = 0 ==> reset Global Interrupt Enable
{
  if (boolean) 
     // write 0x01 in Interrupt Control Register
     writeb(0x01, IP_OptoInput_baseaddress + REG_INTCONT); 
  else
     // write 0x00 in Interrupt Control Register
     writeb(0x00, IP_OptoInput_baseaddress + REG_INTCONT);
}

int IP_OptoInput_Read_GIRF( void )
// Reads the Global Interrupt Request Flag
{
  return(readb(IP_OptoInput_baseaddress + REG_INTCONT) & 0x80);
}

void IP_OptoInput_Write_IERRE(unsigned short writeIERRE)
// Write unsigned short to Interrupt Enable Register Rising Edge
{
  writeShort(writeIERRE,REG_INTENALH);
}

unsigned short IP_OptoInput_Read_IERRE( void )
// Read Interrupt Enable Register Rising Edge
{
  return readShort(REG_INTENALH);
}

void IP_OptoInput_Set_IERRE(unsigned short setIERRE)
// Set some bits of Interrupt Enable Register Rising Edge.
// E.g. setIERRE = 0x010A ==> interrupt for channel 2, 4 and 9 are set
//                            other channels are not changed 
{
  writeShort((readShort(REG_INTENALH) | setIERRE), REG_INTENALH);
}

void IP_OptoInput_Reset_IERRE(unsigned short resetIERRE)
// Reset some bits of Interrupt Enable Register Rising Edge.
// E.g. resetIERRE = 0x010A ==> interrupt for channel 2, 4 and 9 are reset
//                              other channels are not changed
{
  writeShort((readShort(REG_INTENALH) & !resetIERRE), REG_INTENALH);
}


void IP_OptoInput_Write_IERFE(unsigned short writeIERFE)
// Write unsigned short to Interrupt Enable Register Falling Edge
{
  writeShort(writeIERFE,REG_INTENAHL);
}

unsigned short IP_OptoInput_Read_IERFE( void )
// Read Interrupt Enable Register Falling Edge
{
  return readShort(REG_INTENAHL);
}

void IP_OptoInput_Set_IERFE(unsigned short setIERFE)
// Set several bits of Interrupt Enable Register Falling Edge.
// E.g. setIERFE = 0x010A ==> interrupt for channel 2, 4 and 9 are set
//                            other channels are not changed 
{
  writeShort((readShort(REG_INTENAHL) | setIERFE), REG_INTENAHL);
}

void IP_OptoInput_Reset_IERFE(unsigned short resetIERFE)
// Reset some bits of Interrupt Enable Register Falling Edge.
// E.g. resetIERFE = 0x010A ==> interrupt for channel 2, 4 and 9 are reset
//                              other channels are not changed
{
  writeShort((readShort(REG_INTENAHL) & !resetIERFE), REG_INTENAHL);
}

unsigned short IP_OptoInput_Read_ISRRE( void )
// Read Interrupt Status Register Rising Edge.
{
  return readShort(REG_INTSTATLH);
}

void IP_OptoInput_Write_ISRRE(unsigned short writeISRRE)
// Write unsigned short to Interrupt Status Register Rising Edge.
{
  writeShort(writeISRRE,REG_INTSTATLH);
}

void IP_OptoInput_reset_ISRRE(unsigned short resetISRRE)
// Reset some bits of Interrupt Status Register Rising Edge.
// E.g. resetISRRE = 0x010A ==> reset status for channel 2, 4 and 9
//                              other channels are not changed
{
  writeShort(resetISRRE, REG_INTSTATLH);
}

unsigned short IP_OptoInput_Read_ISRFE( void )
// Read Interrupt Status Register Falling Edge.
{
  return readShort(REG_INTSTATHL);
}

void IP_OptoInput_Write_ISRFE(unsigned short writeISRFE)
// Write unsigned short to Interrupt Status Register Falling Edge.
{
  writeShort(writeISRFE,REG_INTSTATHL);
}

void IP_OptoInput_reset_ISRFE(unsigned short resetISRFE)
// Reset some bits of Interrupt Status Register Falling Edge.
// E.g. resetISRRE = 0x010A ==> reset status for channel 2, 4 and 9
//                              other channels are not changed
{
  writeShort(resetISRFE, REG_INTSTATHL);
}

void IP_OptoInput_set_Interrupt_Vector_Register(unsigned short vecNr)
// Sets the vector of the interrupt (just a number?)
{
  writeShort(vecNr, REG_INTVEC);
}

void IP_OptoInput_cleanup (void)
{
#if DEBUG
  printk("IP_OptoInput_cleanup entered\n");
#endif
  // Free the interrupt routine, if assigned
  if (interruptHandle) vme_interrupt_release(VIPC616_GetBusNr(), interruptHandle);
  if (VIPC616_GetIRQ() != 0) free_irq(VIPC616_GetIRQ(), &file_ops);
  
  IP_OptoInput_Set_GIE    (0x0000);
  IP_OptoInput_Write_IERRE(0x0000);
  IP_OptoInput_Write_IERFE(0x0000);
  IP_OptoInput_Write_ISRRE(0xffff);
  IP_OptoInput_Write_ISRFE(0xffff);

  printk(MODULE_NAME " module unloaded\n");
}

int IP_OptoInput_init (void)
// Initialise the IP-OptoInput driver
{
  int result = 0;
#if DEBUG
  printk("IP_OptoInput_init entered\n");
#endif
  
  // 1. Get the board base address
  IP_OptoInput_baseaddress = VIPC616_getCardBaseAddress(MYBOARD, MYSLOT);
  if (!IP_OptoInput_baseaddress)
  {
    printk ("Error: Can't find baseadress for the IP-OptoInput\n");
    return -1;
  }

  // 2. Check if the module is there
  if(IP_Module_check() != 0)
  {
    printk("\n-------------------------------------------------\n");
    printk("Module at position %d%c, it isn't a IP-OptoInput   \n",
            MYBOARD, MYSLOT+'a'-1); 
    printk("Module IP-Encoder 6 will not be loaded!!!\n"); 
    printk("--------------------------------------------------\n\n");
    return -1;
  }

  // 3. Initialise by resetting stuff
  IP_OptoInput_Set_GIE    (0x0001);
  IP_OptoInput_Write_IERRE(0x007f);
  IP_OptoInput_Write_IERFE(0x007f);
  IP_OptoInput_Write_ISRRE(0xFFFF);
  IP_OptoInput_Write_ISRFE(0xFFFF);
  IP_OptoInput_set_Interrupt_Vector_Register(IP_OPTOINPUT_INTERRUPT_VECTOR);

  // 4) Now try to register the interrupt.
  //
  // a) Request the interrupt (it is shared, so this should be OK here?)
  if (0 > (result = request_irq(VIPC616_GetIRQ(),
                                &IP_OptoInput_InterruptHandler, SA_SHIRQ,
                                MODULE_NAME, &file_ops )) )
  {
    printk (KERN_ERR MODULE_NAME ":Failed request_irq, errno: %d\n", result);
    return -1;
  }
  // b)
  /* The VME_INTERRUPT_RESERVE flags tells the VMEBus bridge driver to not 
   * do anything with interrupts on the level we specify here (not just 
   * the level/vector, but the whole level). Therefore, we need to have done
   * the request_irq before attaching, otherwise it may be that noone handles 
   * the interrupt and the system blocks.
   */
  if (0 > (result = vme_interrupt_attach(VIPC616_GetBusNr(), &interruptHandle,
                                         IP_OPTOINPUT_IRQ_LEVEL,
                                         IP_OPTOINPUT_INTERRUPT_VECTOR,
                                         VME_INTERRUPT_RESERVE, NULL) ) )
  {
    printk (KERN_ERR MODULE_NAME ":Failed vme_request_attach, errno: %d\n", result);
    IP_OptoInput_cleanup();
    return -1;
  }
 
  printk(MODULE_NAME " successfully loaded\n");
  
  return 0;
}

module_init(IP_OptoInput_init);
module_exit(IP_OptoInput_cleanup);
