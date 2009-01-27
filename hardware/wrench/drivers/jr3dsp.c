/***************************************************************************
  tag: Johan Rutgeerts  Mon Jan 10 15:59:16 CET 2005  JR3DSP.c 

                        jr3dsp.c -  description
                           -------------------
    begin                : Mon January 10 2005
    copyright            : (C) 2005 Johan Rutgeerts
    email                : johan.rutgeerts@mech.kuleuven.ac.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
 
 
//----------------------------------------------------------------------------- 
// Includes
#include <asm/io.h>  // for writeb/w/l, readb/w/l
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/time.h>

#include "jr3dsp.h"
#include "jr3idm.h"

#ifndef CONFIG_PCI
#   error "This driver needs PCI support to be available"
#endif

MODULE_LICENSE("GPL");


//----------------------------------------------------------------------------- 
// Local variables

static unsigned int num_dsp = 0;

static u8* jr3_baseaddr;

static unsigned short jr3_program[] __devinitdata = JR3DSP_PROGRAM;

static struct pci_device_id jr3pci_id_table[] __devinitdata = {
 {
  vendor:JR3VEND_ID,
  device:JR3DEV_ID_2CHANNEL,
  subvendor:PCI_ANY_ID,
  subdevice:PCI_ANY_ID,
  class:0,
  class_mask:0,
  driver_data:0
 },
 {
  vendor:JR3VEND_ID,
  device:JR3DEV_ID_1CHANNEL,
  subvendor:PCI_ANY_ID,
  subdevice:PCI_ANY_ID,
  class:0,
  class_mask:0,
  driver_data:0
 },
 {0, 0, 0, 0, 0, 0, 0}      
};
MODULE_DEVICE_TABLE(pci, jr3pci_id_table);

//----------------------------------------------------------------------------- 
// Local functions

int JR3DSP_read_word(unsigned int offset, unsigned int dsp)
// Function to read a word off the DSP.
// This function takes the offset from the manual, multiplies it
// by four and adds JR3OFFSET (as described in the appendix).
// dsp is the DSP number (0, 1, 2 or 3).
{
  return readw(jr3_baseaddr + offset * 4 + JR3OFFSET + 0x80000 * dsp);
}



void JR3DSP_write_word(unsigned int offset, unsigned short data, unsigned int dsp)
// Function to write a word to the DSP.
// This function takes the offset from the manual, multiplies it
// by four and adds JR3OFFSET (as described in the appendix). 
// dsp is the DSP number (0, 1, 2 or 3).
{
  writew(data, jr3_baseaddr + offset * 4 + JR3OFFSET + 0x80000 * dsp);
}



//----------------------------------------------------------------------------- 
// Accessible functions functions

RTAI_SYSCALL_MODE void JR3DSP_get_data(struct s16Forces* data, unsigned int filter, unsigned int dsp)
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



RTAI_SYSCALL_MODE void JR3DSP_set_offsets(const struct s16Forces* offsets, unsigned int dsp)
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



RTAI_SYSCALL_MODE u16 JR3DSP_get_error_word(unsigned int dsp)
{
  return JR3DSP_read_word(ERRORS, dsp);
}



RTAI_SYSCALL_MODE u16 JR3DSP_get_command_word0(unsigned int dsp)
{
  return JR3DSP_read_word(COMMAND_WORD0, dsp);
}



RTAI_SYSCALL_MODE u16 JR3DSP_get_units( unsigned int dsp )
// Get units:
// 0: lbs  inLbs  mils
// 1: N    dNm    mmX10
// 2: kgF  kgFcm  mmX10
// 3: klbs kinlbs mils
{
  return JR3DSP_read_word(UNITS, dsp);
}



RTAI_SYSCALL_MODE void JR3DSP_set_units(unsigned int _units, unsigned int dsp)
// Set units:
// 0: lbs  inLbs  mils
// 1: N    dNm    mmX10
// 2: kgF  kgFcm  mmX10
// 3: klbs kinlbs mils
{
  JR3DSP_write_word(UNITS, _units, dsp);
}



RTAI_SYSCALL_MODE void JR3DSP_get_full_scale(struct s16Forces* fullscale, unsigned int dsp)
{
   fullscale->Fx = JR3DSP_read_word( FULL_SCALE+0, dsp );
   fullscale->Fy = JR3DSP_read_word( FULL_SCALE+1, dsp );
   fullscale->Fz = JR3DSP_read_word( FULL_SCALE+2, dsp );
   fullscale->Tx = JR3DSP_read_word( FULL_SCALE+3, dsp );
   fullscale->Ty = JR3DSP_read_word( FULL_SCALE+4, dsp );
   fullscale->Tz = JR3DSP_read_word( FULL_SCALE+5, dsp );
}



static void __devinit JR3DSP_download_code(u8* jr3, int pnum)
{
	if (jr3)
	{
		int pos = 0;
        
		while ( (pos < (sizeof(jr3_program) / sizeof(unsigned short)) )  && (jr3_program[pos] != 0xffff) )
		{
			int count, addr;
			
			count = jr3_program[pos++];
			addr  = jr3_program[pos++];
		  
			while (count > 0)
			{
				if (addr & 0x4000) 
				{
					/* 16 bit data, not used!! */
					int data;

					data = jr3_program[pos++];
					count--;
					
					printk("jr3 DSP data memory, not tested\n");
					writew(data, jr3 + ( addr + 0x20000 * pnum ) * 4 );
				} 
				else
				{
				    /* Download 24 bit program */
					int data1, data2;
                    
					data1 = jr3_program[pos++];
					data2 = jr3_program[pos++];
					count -= 2;
					
					writew(data1, jr3 + (addr           + 0x20000 * pnum ) * 4 );
					writew(data2, jr3 + (addr + 0x10000 + 0x20000 * pnum ) * 4 );
					
				}
				addr++;
			}
		}
	}
}



static int __devinit jr3pci_probe(struct pci_dev* dev, const struct pci_device_id* id)
{
	int i;
	printk("JR3dsp: Initializing pci board.\n\n");

	i = pci_enable_device(dev);
	if (i) 
    {
        printk("pci_enable_device returned: %d", i);
        return i;
    }
    //printk("ioremap\n"); 
	jr3_baseaddr = ioremap( pci_resource_start(dev, 0), pci_resource_len(dev, 0) );
    
	//printk("request_mem_region\n");
	if ( request_mem_region( (unsigned long) jr3_baseaddr, pci_resource_len(dev, 0), "jr3pci") == NULL)
	{	
		printk("mem region 0x%x @ 0x%x busy\n\n", (unsigned int) pci_resource_len(dev, 0), (unsigned int)jr3_baseaddr);
		return -EBUSY;
	}

	printk("JR3dsp: Resetting card.\n");
	writew(0, jr3_baseaddr + 0x18000 * 4 );        // reset the board
	
	printk("JR3dsp: Downloading code for DSP 0.\n");
	JR3DSP_download_code( jr3_baseaddr, 0);
	num_dsp = 1;
	if (id->device == JR3DEV_ID_2CHANNEL){
	  printk("JR3dsp: Downloading code for DSP 1.\n");
	  JR3DSP_download_code( jr3_baseaddr, 1); 
	  num_dsp = 2;
	}
	
	printk("\nJR3dsp: Card initialized.\n\n");
	// The card seems to need some time after a resetting and freshly downloading
	// the code, before it is operational. This waits for 5 seconds and
	// draws nice dots in the mean time to show progress ;)
	//set_current_state(TASK_INTERRUPTIBLE); schedule_timeout( 1 * HZ); 	printk(".");
	//set_current_state(TASK_INTERRUPTIBLE); schedule_timeout( 1 * HZ); 	printk(".");
	//set_current_state(TASK_INTERRUPTIBLE); schedule_timeout( 1 * HZ); 	printk(".");
	//set_current_state(TASK_INTERRUPTIBLE); schedule_timeout( 1 * HZ); 	printk(".");	
	//set_current_state(TASK_INTERRUPTIBLE); schedule_timeout( 1 * HZ); 	printk(".\n\n");
	
	// After waiting, put some general and error info on the screen
	//JR3DSP_check_sensor_and_DSP( 0 );
   	//printk("\n\n");
	//JR3DSP_check_sensor_and_DSP( 1 );
	//printk("\n\n");
	
	return 0;
}


RTAI_SYSCALL_MODE unsigned int JR3DSP_check_sensor_and_DSP( unsigned int dsp )
// Checks for copyright, software date and year on the DSP and the eeprom, 
// software version, serial and model number of the sensor (unique identifiers)
// and calibration date of sensor
{
  unsigned char copyrightTxt[COPYRIGHT_SIZE + 1];
  unsigned int i;
  unsigned short w;

  if (dsp >= num_dsp || dsp < 0){
    printk("DSP nr %i is invalid. Your PCI card only has %i dsp's\n", dsp, num_dsp);
    return 0;
  }
  
  for (i = 0x0; i < COPYRIGHT_SIZE; i++)
  { // The bytes seem to be the character with a space, so the 'C' is stored 
    // int he EEPROM of the receiver card as 'C '. The shift gets rid of
	// the space.
    w = JR3DSP_read_word ( COPYRIGHT + i, dsp) >> 8;
    copyrightTxt[i] = w;
  }

  // Set the final char to 0. If everything is ok, it should be no problem
  // (the last char of the copyright statement is a '0'), but if an error
  // should occur, this solves the problem of an unterminated string
  copyrightTxt[i]=0;
  printk ("\n\n%s\n", copyrightTxt);

  // Print some DSP information
  printk("DSP nr %i\n", dsp);
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




void __devexit jr3pci_remove(struct pci_dev* dev)
{
	printk("jr3pci_remove\n");
	
    printk("release_mem_region\n");
    release_mem_region( (unsigned long) jr3_baseaddr, pci_resource_len(dev, 0));
	
    printk("iounmap\n");
    iounmap(jr3_baseaddr);
    
	printk("pci_disable_device\n");
    pci_disable_device(dev);
}



static struct pci_driver jr3pci_driver =
{
  name:"jr3pci_driver",
  id_table:jr3pci_id_table,
  probe:jr3pci_probe,
  remove:__devexit_p(jr3pci_remove),
  suspend:NULL,
  resume:NULL
};


int __init jr3pci_init_module(void)
{
	//printk("\njr3pci_init_module\n");
	return pci_register_driver(&jr3pci_driver);
}


void __exit jr3pci_cleanup_module(void)
{
	//printk("jr3pci_cleanup_module\n");
	pci_unregister_driver(&jr3pci_driver);
}


module_init(jr3pci_init_module);
module_exit(jr3pci_cleanup_module);

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


