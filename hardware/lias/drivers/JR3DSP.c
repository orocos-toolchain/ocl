// ============================================================================
//
// = KERNEL_MODULE
//    JR3DSP_module.o
//
// = FILENAME
//    JR3DSP.c
//
// = DESCRIPTION
//    Module to implement the communcitation with the JR3 DSP
//
// = AUTHOR
//    Rene Waarsing <Rene.Waarsing@mech.kuleuven.ac.be>
//
// = REFERENCES
//    The JR 3 manual (JR3; dsp-based force sensor receivers; software and
//    installation manual)
// 
// ============================================================================
//
// $Id: JR3DSP.c,v 1.1.2.4 2003/08/18 16:06:56 rwaarsin Exp $
// $Log: JR3DSP.c,v $
// Revision 1.1.2.4  2003/08/18 16:06:56  rwaarsin
// Changed to new version of kernel, thus a new version of the Universe module + rtlinux -> kernel
//
// Revision 1.1.2.2  2003/01/27 14:36:02  rwaarsin
// ...
//
// Revision 1.1.2.1  2002/07/19 14:42:08  rwaarsin
// Initial release
//
//
// ============================================================================

//----------------------------------------------------------------------------- 
// Includes
#include <asm/io.h>  // for writeb/w/l, readb/w/l
#include <asm/uaccess.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/time.h>

#include "JR3DSP.h"


#include <vme/vme_api.h>
#include <vme/vme.h>


MODULE_LICENSE("GPL");
#define MODULE_NAME "JR3DSP_VME"


//----------------------------------------------------------------------------- 
// Local variables

vme_bus_handle_t    JR3DSP_BusHandle = 0;
int                 JR3DSP_VMEBusBridgeIrq = 0;

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

struct VmeWindowDef jr3dsp =
{
  VmeAddress:0x018000,
  AddressModifier:VME_A24UD,
  WindowSize:0x600,
  Flags:VME_CTL_MAX_DW_32,
  PhysAddr:0,
  // To make sure they're initialise properly
  ptr:0,
  windowHandle:0
};


//----------------------------------------------------------------------------- 
// Local functions

int JR3DSP_read_word(unsigned int offset, unsigned int dsp)
// Function te read a word off the DSP.
// This function takes the offset from the manual and just multiplies it
// by two (the offset in the manual are for word access)
{
  return readw(jr3dsp.ptr + offset * 2);
}




void JR3DSP_write_word(unsigned int offset, unsigned short data, unsigned int dsp)
// Function te write a word to the DSP.
// This function takes the offset from the manual and just multiplies it
// by two (the offset in the manual are for word access)
{
   writew(data, jr3dsp.ptr + offset * 2);
}




//----------------------------------------------------------------------------- 
// Accessible functions functions

void JR3DSP_get_data(struct s16Forces* data, unsigned int filter, unsigned int dsp)
// Function to get the data from a filter into the array.
{
    unsigned int filteroffset = FILTER0;
    switch (filter)
    {
        case 0: filteroffset = FILTER0; break;
        case 1: filteroffset = FILTER1; break;
        case 2: filteroffset = FILTER2; break;
        case 3: filteroffset = FILTER3; break;
        case 4: filteroffset = FILTER4; break;
        case 5: filteroffset = FILTER5; break;
        case 6: filteroffset = FILTER6; break;
    }
    
    data->Fx = JR3DSP_read_word( filteroffset + 0, dsp );
    data->Fy = JR3DSP_read_word( filteroffset + 1, dsp );
    data->Fz = JR3DSP_read_word( filteroffset + 2, dsp );
    data->Tx = JR3DSP_read_word( filteroffset + 3, dsp );
    data->Ty = JR3DSP_read_word( filteroffset + 4, dsp );
    data->Tz = JR3DSP_read_word( filteroffset + 5, dsp );
}



void JR3DSP_set_offsets(const struct s16Forces* offsets, unsigned int dsp)
{
  JR3DSP_write_word(OFFSETS+0, offsets->Fx, dsp);
  JR3DSP_write_word(OFFSETS+1, offsets->Fy, dsp);
  JR3DSP_write_word(OFFSETS+2, offsets->Fz, dsp);
  JR3DSP_write_word(OFFSETS+3, offsets->Tx, dsp);
  JR3DSP_write_word(OFFSETS+4, offsets->Ty, dsp);
  JR3DSP_write_word(OFFSETS+5, offsets->Tz, dsp);

  // And immediately change the offset
  //JR3DSP_write_word(COMMAND_WORD0, 0x0700, dsp);
}



u16 JR3DSP_get_error_word(unsigned int dsp)
{
  return JR3DSP_read_word(ERRORS, dsp);
}



u16 JR3DSP_get_command_word0(unsigned int dsp)
{
  return JR3DSP_read_word(COMMAND_WORD0, dsp);
}



u16 JR3DSP_get_units( unsigned int dsp )
// Get units:
// 0: lbs  inLbs  mils
// 1: N    dNm    mmX10
// 2: kgF  kgFcm  mmX10
// 3: klbs kinlbs mils
{
  return JR3DSP_read_word(UNITS, dsp);
}



void JR3DSP_set_units(unsigned int _units, unsigned int dsp)
// Set units:
// 0: lbs  inLbs  mils
// 1: N    dNm    mmX10
// 2: kgF  kgFcm  mmX10
// 3: klbs kinlbs mils
{
  JR3DSP_write_word(UNITS, _units, dsp);
}



void JR3DSP_get_full_scale(struct s16Forces* fullscale, unsigned int dsp)
{
   fullscale->Fx = JR3DSP_read_word( FULL_SCALE+0, dsp );
   fullscale->Fy = JR3DSP_read_word( FULL_SCALE+1, dsp );
   fullscale->Fz = JR3DSP_read_word( FULL_SCALE+2, dsp );
   fullscale->Tx = JR3DSP_read_word( FULL_SCALE+3, dsp );
   fullscale->Ty = JR3DSP_read_word( FULL_SCALE+4, dsp );
   fullscale->Tz = JR3DSP_read_word( FULL_SCALE+5, dsp );
}

u16 JR3DSP_check_sensor_and_DSP(unsigned int dsp)
// Checks for copyright, software date and year on the DSP and the eeprom, 
// software version, serial and model number of the sensor (unique identifiers)
// and calibration date of sensor
{
  unsigned char copyrightTxt[COPYRIGHT_SIZE + 1];
  unsigned int i;
  unsigned short w;
  
  for (i = 0x0; i < COPYRIGHT_SIZE; i++)
  { // The bytes seem to be the character with a space, so the 'C' is stored 
    // int he EEPROM of the receiver card as 'C '. Because we can only use 
    // word read, we use the shift
    w = JR3DSP_read_word ( COPYRIGHT + i, dsp) >> 8;
    copyrightTxt[i] = w;
  }

  // Set the final char to 0. If everything is ok, it should be no problem
  // (the last char of the copyright statement is a '0'), but if an error
  // should occur, this solves the problem of an unterminated string
  copyrightTxt[i]=0;
  printk ("%s\n", copyrightTxt);

  // Print some DSP information
  printk("Not using DSP nr.");
  printk("Software version nr: %d.%02d (released on day %d of %d)\n",
  JR3DSP_read_word ( SOFTWARE_VER_NO, dsp )/100,
  JR3DSP_read_word ( SOFTWARE_VER_NO, dsp )%100,
  JR3DSP_read_word ( SOFTWARE_DAY, dsp ), JR3DSP_read_word ( SOFTWARE_YEAR, dsp ));

  // Print some information from the connected sensor
  printk("Information from the attached sensor:\n");
  printk("  Eeprom version nr:   %d\n",
    JR3DSP_read_word ( EEPROM_VER_NO, dsp ));
  printk("  Serial nr:           %d\n",
    JR3DSP_read_word ( SERIAL_NO, dsp ));
  printk("  Model nr:            %d\n",
    JR3DSP_read_word ( MODEL_NO, dsp ));
  printk("  Thickness:           %d ", JR3DSP_read_word ( THICKNESS, dsp ));
  printk("  Calibration:         day %d of %d\n",
    JR3DSP_read_word ( CAL_DAY, dsp ), JR3DSP_read_word ( CAL_YEAR, dsp ));

  if (JR3DSP_read_word(ERRORS, dsp) != 0)
  {
    printk("There seem to be errors!\n");
    printk("Error code : 0x%x\n", JR3DSP_read_word(ERRORS, dsp));
    printk("Check the manual for further explanation.\n");
    return 0;
  }
  return 1;
}



void JR3DSP_cleanup(void)
{
  if (jr3dsp.ptr)
    vme_master_window_unmap(JR3DSP_BusHandle, jr3dsp.windowHandle);
  if (jr3dsp.windowHandle)
    vme_master_window_release(JR3DSP_BusHandle, jr3dsp.windowHandle);
  if(JR3DSP_BusHandle) vme_term(JR3DSP_BusHandle);
  printk("VME/Board module unloaded\n");
}



int JR3DSP_init(void)
{
  int result;

  // 1. Map the VME Memory
  // 0. Get the vme_handle
  if (0 > (result = vme_init(&JR3DSP_BusHandle)))
  {
    printk (KERN_ERR MODULE_NAME ":Failed vme_init, errno= %d\n", result);
    return -1;
  }
  // 1. Map the VME Memory

  if ( 0 > (result = vme_master_window_create (JR3DSP_BusHandle, &jr3dsp.windowHandle,
            jr3dsp.VmeAddress, jr3dsp.AddressModifier,
            jr3dsp.WindowSize, jr3dsp.Flags,
            jr3dsp.PhysAddr) ) )
  {
    printk (KERN_ERR MODULE_NAME ":Failed to create vme window errno= %d\n", result);
    JR3DSP_cleanup();
  }

  jr3dsp.ptr = vme_master_window_map(JR3DSP_BusHandle, jr3dsp.windowHandle, 0);
   
  return 0;
}





































































































module_init (JR3DSP_init);
module_exit (JR3DSP_cleanup);

//----------------------------------------------------------------------------- 
// The symbols that are available outside this module
EXPORT_SYMBOL (JR3DSP_check_sensor_and_DSP);
EXPORT_SYMBOL (JR3DSP_get_error_word);
EXPORT_SYMBOL (JR3DSP_get_command_word0);
EXPORT_SYMBOL (JR3DSP_get_units);
EXPORT_SYMBOL (JR3DSP_set_units);
EXPORT_SYMBOL (JR3DSP_set_offsets);
EXPORT_SYMBOL (JR3DSP_get_data);
EXPORT_SYMBOL (JR3DSP_get_full_scale);


