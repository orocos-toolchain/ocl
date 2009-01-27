/***************************************************************************
  tag: Johan Rutgeerts  Mon Jan 10 15:59:16 CET 2005  JR3DSP.h 

                        jr3dsp.h -  description
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
 
 
#ifndef JR3DSP_H
#define JR3DSP_H

#include <rtai_lxrt.h>

struct s16Forces
{
    s16   Fx, Fy, Fz, Tx, Ty, Tz; // Signed 16 bit
};


RTAI_SYSCALL_MODE unsigned int JR3DSP_check_sensor_and_DSP( unsigned int dsp );

RTAI_SYSCALL_MODE u16  JR3DSP_get_error_word(unsigned int dsp);

RTAI_SYSCALL_MODE u16  JR3DSP_get_command0(unsigned int dsp);

RTAI_SYSCALL_MODE u16  JR3DSP_get_units( unsigned int dsp );

RTAI_SYSCALL_MODE void JR3DSP_set_units( unsigned int type, unsigned int dsp);

RTAI_SYSCALL_MODE void JR3DSP_set_offsets(const struct s16Forces* offsets, unsigned int dsp);

RTAI_SYSCALL_MODE void JR3DSP_get_data(struct s16Forces* data, unsigned int filter, unsigned int dsp);

RTAI_SYSCALL_MODE void JR3DSP_get_full_scale(struct s16Forces* data, unsigned int dsp);



// defines

#define COPYRIGHT_SIZE     0x18

// All the offsets (see manual pp 7- 18 and pp 47)
#define RAW_CHANNELS       0x0000
#define COPYRIGHT          0x0040
#define RESERVED1          0x0058
#define SHUNTS             0x0060
#define RESERVED2          0x0066
#define DEFAULT_FS         0x0068
#define RESERVED3          0x006e
#define LOAD_ENVELOPE_NUM  0x006f
#define MIN_FULL_SCALE     0x0070
#define RESERVED4          0x0076
#define TRANSFORM_NUM      0x0077
#define MAX_FULL_SCALE     0x0078
#define RESERVED5          0x007e
#define PEAK_ADDRESS       0x007f
#define FULL_SCALE         0x0080
#define OFFSETS            0x0088
#define OFFSET_NUM         0x008e
#define VECT_AXES          0x008f
#define FILTER0            0x0090
#define FILTER1            0x0098
#define FILTER2            0x00a0
#define FILTER3            0x00a8
#define FILTER4            0x00b0
#define FILTER5            0x00b8
#define FILTER6            0x00c0
#define RATE_DATA          0x00c8
#define MINIMUM_DATA       0x00d0
#define MAXIMUM_DATA       0x00d8
#define NEAR_SAT_VALUE     0x00e0
#define SAT_VALUE          0x00e1
#define RATE_ADDRESS       0x00e2
#define RATE_DIVISOR       0x00e3
#define RATE_COUNT         0x00e4
#define COMMAND_WORD2      0x00e5
#define COMMAND_WORD1      0x00e6
#define COMMAND_WORD0      0x00e7
#define COUNT1             0x00e8
#define COUNT2             0x00e9
#define COUNT3             0x00ea
#define COUNT4             0x00eb
#define COUNT5             0x00ec
#define COUNT6             0x00ed
#define ERROR_COUNT        0x00ee
#define COUNT_X            0x00ef
#define WARNINGS           0x00f0
#define ERRORS             0x00f1
#define THRESHOLD_BITS     0x00f2
#define LAST_CRC           0x00f3
#define EEPROM_VER_NO      0x00f4
#define SOFTWARE_VER_NO    0x00f5
#define SOFTWARE_DAY       0x00f6
#define SOFTWARE_YEAR      0x00f7
#define SERIAL_NO          0x00f8
#define MODEL_NO           0x00f9
#define CAL_DAY            0x00fa
#define CAL_YEAR           0x00fb
#define UNITS              0x00fc
#define BITS               0x00fd
#define CHANNELS           0x00fe
#define THICKNESS          0x00ff
#define LOAD_ENVELOPES     0x0100
#define TRANSFORMS         0x0200
#define JR3OFFSET          0x10000

#define JR3VEND_ID         0x1762   /* JR3 Vendor ID */

/* 0x3112: Device ID double channel PCI (for 2 devices or 12DoF)
 * 0x1111: Device ID single channel PCI 
 * 0x211n: Device ID double channel Compact PCI */
#define JR3DEV_ID_2CHANNEL          0x3112   
#define JR3DEV_ID_1CHANNEL          0x1111

#endif // JR3DSP_H
