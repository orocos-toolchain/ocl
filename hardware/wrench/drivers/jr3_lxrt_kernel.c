/***************************************************************************
  tag: Johan Rutgeerts  Mon Jan 10 15:59:16 CET 2005  jr3dsp_lxrt.h 

                        jr3dsp_lxrt.h -  description
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
 
#include <linux/module.h>
#include <rtai_lxrt.h>
#include "jr3_lxrt_common.h"


MODULE_LICENSE("GPL");


RTAI_SYSCALL_MODE extern unsigned int JR3DSP_check_sensor_and_DSP( unsigned int dsp );
RTAI_SYSCALL_MODE extern u16  JR3DSP_get_error_word(unsigned int dsp);
RTAI_SYSCALL_MODE extern u16  JR3DSP_get_command_word0(unsigned int dsp);
RTAI_SYSCALL_MODE extern u16  JR3DSP_get_units( unsigned int dsp );
RTAI_SYSCALL_MODE extern void JR3DSP_set_units( unsigned int type, unsigned int dsp);
RTAI_SYSCALL_MODE extern void JR3DSP_set_offsets(const struct s16Forces* offsets, unsigned int dsp);
RTAI_SYSCALL_MODE extern void JR3DSP_get_data(struct s16Forces* data, unsigned int filter, unsigned int dsp);
RTAI_SYSCALL_MODE extern void JR3DSP_get_full_scale(struct s16Forces* data, unsigned int dsp);


static struct rt_fun_entry rt_jr3_fun[] = {
    [ JR3DSP_CHECK_SENSOR_AND_DSP ] = { 0, JR3DSP_check_sensor_and_DSP},
    [ JR3DSP_GET_ERROR_WORD       ] = { 0, JR3DSP_get_error_word},
    [ JR3DSP_GET_UNITS            ] = { 0, JR3DSP_get_units},
    [ JR3DSP_SET_UNITS            ] = { 0, JR3DSP_set_units},
    [ JR3DSP_SET_OFFSETS          ] = { UR1(1,3), JR3DSP_set_offsets},
    [ JR3DSP_GET_DATA             ] = { UW1(1,4), JR3DSP_get_data},
    [ JR3DSP_GET_FULL_SCALE       ] = { UW1(1,3), JR3DSP_get_full_scale},
    [ JR3DSP_GET_COMMAND_WORD0    ] = { 0, JR3DSP_get_command_word0}
};



int init_module(void)
{

	if( set_rt_fun_ext_index(rt_jr3_fun, MYIDX))
    {
		printk("Recompile your module with a different index\n");
		return -EACCES;
	}
    
	return(0);
}


void cleanup_module(void)
{
	reset_rt_fun_ext_index(rt_jr3_fun, MYIDX);
}

