
#include <linux/module.h>
#include <rtai_lxrt.h>
#include "LiAS_lxrt_common.h"


MODULE_LICENSE("GPL");


extern void JR3DSP_check_sensor_and_DSP( unsigned int dsp );
extern u16  JR3DSP_get_error_word(unsigned int dsp);
extern u16  JR3DSP_get_command_word0(unsigned int dsp);
extern u16  JR3DSP_get_units( unsigned int dsp );
extern void JR3DSP_set_units( unsigned int type, unsigned int dsp);
extern void JR3DSP_set_offsets(const struct s16Forces* offsets, unsigned int dsp);
extern void JR3DSP_get_data(struct s16Forces* data, unsigned int filter, unsigned int dsp);
extern void JR3DSP_get_full_scale(struct s16Forces* data, unsigned int dsp);
extern int  IP_Digital_24_set_bit_of_channel   (unsigned int outputline);
extern int  IP_Digital_24_clear_bit_of_channel (unsigned int outputline);
extern char IP_Digital_24_get_bit_of_channel  (unsigned int inputline);
extern unsigned short IP_Encoder_6_get_counter_channel (unsigned int registerNo);
extern void IP_FastDAC_enable_dac (void);
extern int  IP_FastDAC_set_gain_of_group(char group, char gain);
extern int  IP_FastDAC_write_to_channel (unsigned short value, char channel);
extern unsigned short IP_OptoInput_Read_IDR( void );



static struct rt_fun_entry rt_LiAS_fun[] = {
    [ JR3DSP_CHECK_SENSOR_AND_DSP        ] = { 0, JR3DSP_check_sensor_and_DSP},
    [ JR3DSP_GET_ERROR_WORD              ] = { 0, JR3DSP_get_error_word},
    [ JR3DSP_GET_UNITS                   ] = { 0, JR3DSP_get_units},
    [ JR3DSP_SET_UNITS                   ] = { 0, JR3DSP_set_units},
    [ JR3DSP_SET_OFFSETS                 ] = { UR1(1,3), JR3DSP_set_offsets},
    [ JR3DSP_GET_DATA                    ] = { UW1(1,4), JR3DSP_get_data},
    [ JR3DSP_GET_FULL_SCALE              ] = { UW1(1,3), JR3DSP_get_full_scale},
    [ JR3DSP_GET_COMMAND_WORD0           ] = { 0, JR3DSP_get_command_word0},
    [ IP_ENCODER_6_GET_COUNTER_CHANNEL   ] = { 0, IP_Encoder_6_get_counter_channel},
    [ IP_DIGITAL_24_SET_BIT_OF_CHANNEL   ] = { 0, IP_Digital_24_set_bit_of_channel},
    [ IP_DIGITAL_24_CLEAR_BIT_OF_CHANNEL ] = { 0, IP_Digital_24_clear_bit_of_channel},
    [ IP_DIGITAL_24_GET_BIT_OF_CHANNEL   ] = { 0, IP_Digital_24_get_bit_of_channel},
    [ IP_FASTDAC_ENABLE_DAC              ] = { 0, IP_FastDAC_enable_dac},
    [ IP_FASTDAC_SET_GAIN_OF_GROUP       ] = { 0, IP_FastDAC_set_gain_of_group},
    [ IP_FASTDAC_WRITE_TO_CHANNEL        ] = { 0, IP_FastDAC_write_to_channel},
    [ IP_OPTOINPUT_READ_IDR              ] = { 0, IP_OptoInput_Read_IDR}
};


/* init module */
int init_module(void)
{
  if( set_rt_fun_ext_index(rt_LiAS_fun, MYIDX))
  {
    printk("Recompile the LiAS lxrt module with a different index\n");
   return -EACCES;
  }

  return(0);
}

/*  cleanup module */
void cleanup_module(void)
{
  reset_rt_fun_ext_index(rt_LiAS_fun, MYIDX);
}


