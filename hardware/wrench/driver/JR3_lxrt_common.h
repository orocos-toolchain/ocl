#ifndef JR3_LXRT_COMMON_H
#define JR3_LXRT_COMMON_H


#include <asm/types.h>
#ifndef __KERNEL__
    #define s16 __s16
#endif

struct s16Forces
{
    s16   Fx, Fy, Fz, Tx, Ty, Tz; // Signed 16 bit
};


#define RANGE_5_V    1
#define RANGE_10_V   0

/** These are used somewhere else for LiAS :
#define MYIDX			15
#define IP_ENCODER_6_GET_COUNTER_CHANNEL     1
#define IP_DIGITAL_24_SET_BIT_OF_CHANNEL     2
#define IP_DIGITAL_24_CLEAR_BIT_OF_CHANNEL   3
#define IP_DIGITAL_24_GET_BIT_OF_CHANNEL     4
#define IP_FASTDAC_ENABLE_DAC                5
#define IP_FASTDAC_SET_GAIN_OF_GROUP         6
#define IP_FASTDAC_WRITE_TO_CHANNEL          7
#define IP_OPTOINPUT_READ_IDR                8
*/
#define MYIDX			                    14
#define JR3DSP_CHECK_SENSOR_AND_DSP         60
#define JR3DSP_GET_ERROR_WORD               61
#define JR3DSP_GET_UNITS                    62
#define JR3DSP_SET_UNITS                    63
#define JR3DSP_SET_OFFSETS                  64
#define JR3DSP_GET_DATA                     65
#define JR3DSP_GET_FULL_SCALE               66
#define JR3DSP_GET_COMMAND_WORD0            67

#endif

