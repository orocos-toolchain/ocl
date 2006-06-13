#ifndef LiAS_LXRT_USER_H
#define LiAS_LXRT_USER_H


#include <rtai_lxrt.h>
#include "LiAS_lxrt_common.h"


static inline int IP_Digital_24_set_bit_of_channel(unsigned int outputline)
{
  unsigned short retval;
  struct {unsigned int ouputline; } arg = { outputline };
  retval = rtai_lxrt(MYIDX, SIZARG, IP_DIGITAL_24_SET_BIT_OF_CHANNEL, &arg).i[LOW];
  return retval;
}

static inline int IP_Digital_24_clear_bit_of_channel (unsigned int outputline)
{
  unsigned short retval;
  struct {unsigned int outputline; } arg = { outputline };
  retval = rtai_lxrt(MYIDX, SIZARG, IP_DIGITAL_24_CLEAR_BIT_OF_CHANNEL, &arg).i[LOW];
  return retval;
}

static inline char IP_Digital_24_get_bit_of_channel  (unsigned int inputline)
{
  unsigned short retval;
  struct {unsigned int inputline; } arg = { inputline };
  retval = rtai_lxrt(MYIDX, SIZARG, IP_DIGITAL_24_GET_BIT_OF_CHANNEL, &arg).i[LOW];
  return retval;
}

static inline unsigned short IP_Encoder_6_get_counter_channel (unsigned int registerNo)
{
  unsigned short retval;
  struct {unsigned int registerNo; } arg = { registerNo };
  retval = rtai_lxrt(MYIDX, SIZARG, IP_ENCODER_6_GET_COUNTER_CHANNEL, &arg).i[LOW];
  return retval;
}

static inline void IP_FastDAC_enable_dac ()
{
  struct {unsigned int dummy; } arg = { 0 };
  rtai_lxrt(MYIDX, SIZARG, IP_FASTDAC_ENABLE_DAC , &arg);
}

static inline int IP_FastDAC_set_gain_of_group(unsigned int group, unsigned int gain)
{
  unsigned short retval;
  struct {unsigned int group; unsigned int gain; } arg = { group, gain };
  retval = rtai_lxrt(MYIDX, SIZARG, IP_FASTDAC_SET_GAIN_OF_GROUP, &arg).i[LOW];
  return retval;
}

static inline int  IP_FastDAC_write_to_channel (unsigned int value, unsigned int channel)
{
   //printf("IP_FastDAC_write_to_channel: %i, %i\n", value, channel);
  unsigned short retval;
  struct { unsigned int value; unsigned int channel; } arg = { value, channel };
  retval = rtai_lxrt(MYIDX, SIZARG, IP_FASTDAC_WRITE_TO_CHANNEL, &arg).i[LOW];
  return retval;
}

static inline int  IP_FastDAC_write_float_to_channel (float value, char channel)
{
    //CAVEAT! This only works with the gains set to -10 / 10V
    
  // 1. Check the range
  if (value < -10) value = -10; 
  if (value >  10) value =  10; 

  // 2. Scale the value;
  value /= 10;

  // 3. Return the short 13bit value (cfr. 2-4 & 2-8)  
  return IP_FastDAC_write_to_channel(((short) ((value * 4095.0) + 4096)), channel);
}

static inline unsigned short IP_OptoInput_Read_IDR( )
{
  unsigned short retval;
  struct {char dummy; } arg = { 0 };
  retval = rtai_lxrt(MYIDX, SIZARG, IP_OPTOINPUT_READ_IDR, &arg).i[LOW];
  return retval;
}

static inline void JR3DSP_check_sensor_and_DSP( unsigned int dsp )
{
  struct { unsigned int dsp; } arg = { dsp };
  rtai_lxrt(MYIDX, SIZARG, JR3DSP_CHECK_SENSOR_AND_DSP, &arg);
}

static inline unsigned int JR3DSP_get_error_word(unsigned int dsp)
{
  unsigned int retval;
  struct { unsigned int dsp; } arg = { dsp };
  retval = rtai_lxrt(MYIDX, SIZARG, JR3DSP_GET_ERROR_WORD, &arg).i[LOW];
  return retval;
}

static inline unsigned int JR3DSP_get_command_word0(unsigned int dsp)
{
  unsigned int retval;
  struct { unsigned int dsp; } arg = { dsp };
  retval = rtai_lxrt(MYIDX, SIZARG, JR3DSP_GET_COMMAND_WORD0, &arg).i[LOW];
  return retval;
}

static inline unsigned int JR3DSP_get_units(unsigned int dsp)
{
  unsigned int retval;
  struct { unsigned int dsp; } arg = { dsp };
  retval = rtai_lxrt(MYIDX, SIZARG, JR3DSP_GET_UNITS, &arg).i[LOW];
  return retval;
}

static inline void JR3DSP_set_units(unsigned int type, unsigned int dsp)
{
  struct { unsigned int type; unsigned int dsp; } arg = { type, dsp };
  rtai_lxrt(MYIDX, SIZARG, JR3DSP_SET_UNITS, &arg);
}

static inline void JR3DSP_set_offsets(const struct s16Forces* offsets, unsigned int dsp)
{
    struct s16Forces offs;
    memcpy(&offs, offsets, sizeof(struct s16Forces));
    struct { struct s16Forces* offsts; unsigned int dsp; unsigned int size; } arg = {&offs, dsp, sizeof(struct s16Forces)};
    rtai_lxrt(MYIDX, SIZARG, JR3DSP_SET_OFFSETS, &arg);
}

static inline void JR3DSP_get_data(struct s16Forces* data, unsigned int filter, unsigned int dsp)
{
  struct s16Forces dat;
  struct { struct s16Forces* data; unsigned int filter; unsigned int dsp; unsigned int size;} arg = { &dat, filter, dsp, sizeof(struct s16Forces) };
  rtai_lxrt(MYIDX, SIZARG, JR3DSP_GET_DATA, &arg);
  memcpy(data, &dat, sizeof(struct s16Forces) );
}

static inline void JR3DSP_get_full_scale(struct s16Forces* data, unsigned int dsp)
{
  struct s16Forces dat;
  struct { struct s16Forces* data; unsigned int dsp; unsigned int size;} arg = { &dat, dsp, sizeof(struct s16Forces) };
  rtai_lxrt(MYIDX, SIZARG, JR3DSP_GET_FULL_SCALE, &arg);
  memcpy(data, &dat, sizeof(struct s16Forces) );
}

#endif
