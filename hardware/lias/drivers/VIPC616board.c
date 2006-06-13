// ============================================================================
//
// = KERNEL_MODULE
//    VIPC616board_module.o
//
// = FILENAME
//    VIPC616board.c
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
// $Id: VIPC616board.c,v 1.1.2.2 2003/08/18 16:07:22 rwaarsin Exp $
// $Log: VIPC616board.c,v $
// Revision 1.1.2.2  2003/08/18 16:07:22  rwaarsin
// Changed to new version of kernel, thus a new version of the Universe module
//
//
// ============================================================================

#define DEBUG 0

//----------------------------------------------------------------------------- 
// Includes
#include "VIPC616board.h"

#include <asm/io.h>  // for writeb/w/l, readb/w/l
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>

#include <vme/vme.h>

MODULE_LICENSE("GPL");
#define MODULE_NAME "VIPC616board-rt.o"

//----------------------------------------------------------------------------- 
// Local defines
#define NUMBER_OF_CARRIER_BOARDS 2
#define MAXIMUM_NR_CARDS         4
#define MAXIMUM_INTERRUPTS       1

//----------------------------------------------------------------------------- 
// Local variables
vme_bus_handle_t    VIPC616_BusHandle = 0;
int                 VIPC616_VMEBusBridgeIrq = 0;

struct VmeWindowDef
{
  vme_master_handle_t windowHandle; 
  uint64_t VmeAddress;
  int      AddressModifier;
  size_t   WindowSize;
  int      Flags;
  void*    PhysAddr;
  char*    ptr;
};

static struct VmeWindowDef ipCarrierboard[NUMBER_OF_CARRIER_BOARDS] =
{
  {
    VmeAddress:0x8000,
    AddressModifier:VME_A16S,
    WindowSize:0x400,
    Flags:VME_CTL_MAX_DW_32,
    PhysAddr:0,
    // To make sure they're initialise properly
    ptr:0,
    windowHandle:0
  },
  {
    VmeAddress:0x9000,
    AddressModifier:VME_A16S,
    WindowSize:0x400,
    Flags:VME_CTL_MAX_DW_32,
    PhysAddr:0,
    // To make sure they're initialise properly
    ptr:0,
    windowHandle:0
  }
};

//----------------------------------------------------------------------------- 
// The symbols that are availble outside this module
EXPORT_SYMBOL (VIPC616_getCardBaseAddress);
EXPORT_SYMBOL (VIPC616_IP_Module_check);
EXPORT_SYMBOL (VIPC616_GetIRQ);
EXPORT_SYMBOL (VIPC616_GetBusNr);

//----------------------------------------------------------------------------- 
// Global functions

int VIPC616_GetIRQ(void)
// Returns the irq of the VMEBus bridge
{
  return VIPC616_VMEBusBridgeIrq;
}

vme_bus_handle_t VIPC616_GetBusNr(void)
// Returns the handle of the VMEBus bridge
{
  return VIPC616_BusHandle;
}

unsigned char* VIPC616_getCardBaseAddress(int boardNr, int slotNr)
// Return the pointer to the base address of the requested slot
// Return 0 if the board or slot doesn't exist
// ! boardNr = 1, 2, ...
// ! slotNr  = 1, 2, ...
{
  // Check the validity
  if ((boardNr < 0) || (boardNr > NUMBER_OF_CARRIER_BOARDS)) return NULL;
  if ((slotNr  < 0) || (boardNr > MAXIMUM_NR_CARDS))         return NULL;
  
  return (char*)(ipCarrierboard[boardNr-1].ptr + (slotNr-1)*0x100);
}


int VIPC616_IP_Module_check (unsigned char* boardAddr, unsigned char* IDProm,
                             unsigned int IDProm_size, int swapendian)
{
  unsigned char character; 
  int i;

  for (i = 0; i < IDProm_size; i++)
  {
    if (swapendian)
      character = *(boardAddr + 0x80 + 2*i);
    else
      character = *(boardAddr + 0x81 + 2*i);

#if DEBUG
    printk ("[%d] : 0x%x (should be 0x%x)\n", i, character, IDProm[i]);
#endif
    if (character != IDProm[i])
      return i+1;
  }
  return 0;
}

//----------------------------------------------------------------------------- 
// Local functions

void VIPC616_print_board_info(int boardNr, unsigned char* board_base_addr)
// Checks for IP cards at the slots of the card
{
  int  i;
  char letterCounter = 'a';
  unsigned char* cardPointer = board_base_addr;

  printk("VME/Board %d module successfully mapped to pcispace\n", boardNr);	
  for (i=0; i<MAXIMUM_NR_CARDS; i++, letterCounter++)
  {
    printk ("..Slot %d%c: ", boardNr, letterCounter);
    if ( (*(cardPointer+0x81) == 'I')
      && (*(cardPointer+0x83) == 'P')
      && (*(cardPointer+0x85) == 'A') )
      printk ("occupied\n");
    else
      printk ("empty\n");
    cardPointer += 0x100;
  }
  printk("Beware! This list may not be exhaustive, because certain \n");
  printk("ID PROM's may only read once after reset!\n\n");
}

void VIPC616_cleanup(void)
{
  int i;
  
  for (i=0; i<NUMBER_OF_CARRIER_BOARDS; i++)
  {
    if (ipCarrierboard[i].ptr)
      vme_master_window_unmap(VIPC616_BusHandle, ipCarrierboard[i].windowHandle);
    if (ipCarrierboard[i].windowHandle)
      vme_master_window_release(VIPC616_BusHandle, ipCarrierboard[i].windowHandle);
  }
  if(VIPC616_BusHandle) vme_term(VIPC616_BusHandle);
  printk("VME/Board module unloaded\n");
}

int VIPC616_init(void)
{
  int i, result;

  // 0. Get the vme_handle
  if (0 > (result = vme_init(&VIPC616_BusHandle)))
  {
    printk (KERN_ERR MODULE_NAME ":Failed vme_init, errno= %d\n", result);
    return -1;
  }
  // 1. Map the VME Memory
  // a) print setup
    
#if DEBUG
  printk("\nDriver setup:\n\n");
  for (i=0;i< NUMBER_OF_CARRIER_BOARDS;i++)
  {    
    printk("  >> IP Carrier %d at 0x%X with size 0x%x \n",
      i, (unsigned int)ipCarrierboard[i].VmeAddress, (unsigned int)ipCarrierboard[i].WindowSize);
  }
#endif

  // b) Create the handle and map them

  for (i=0; i<NUMBER_OF_CARRIER_BOARDS; i++)
  {
    if ( 0 > (result = vme_master_window_create (VIPC616_BusHandle, &ipCarrierboard[i].windowHandle,
              ipCarrierboard[i].VmeAddress,  ipCarrierboard[i].AddressModifier,
              ipCarrierboard[i].WindowSize, ipCarrierboard[i].Flags,
              ipCarrierboard[i].PhysAddr) ) )
   {
     printk (KERN_ERR MODULE_NAME ":Failed to create vme window %d, errno= %d\n",i, result);
     VIPC616_cleanup();
   }

   ipCarrierboard[i].ptr = vme_master_window_map(VIPC616_BusHandle, ipCarrierboard[i].windowHandle, 0);
    
   VIPC616_print_board_info(i+1,
      (unsigned char*)ipCarrierboard[i].ptr);
  }

  // get the IRQ of the VME bridge
  if ( 0 > (result = vme_interrupt_irq(VIPC616_BusHandle, &VIPC616_VMEBusBridgeIrq)) )
  {
    printk(KERN_ERR MODULE_NAME ": failed vme_interrupt_req, errno: %d\n", result);
    VIPC616_cleanup();
  }
  

#if 0
  // Initialise the variables containing the registered interrupts:
  for (i = 0; i< MAXIMUM_INTERRUPTS; i++)
  {
    registeredInterrupts[i].interruptHandler = 0;
    registeredInterrupts[i].irqLevel         = 0;
    registeredInterrupts[i].irqVector        = 0;
  };
  // Assign a handler to the Interrupt
  VIPC616_InstallUniverseInterruptHandler(1);
#endif
  return 0;
}

module_init (VIPC616_init);
module_exit (VIPC616_cleanup);

