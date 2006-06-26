#ifndef JR3_LXRT_USER_H
#define JR3_LXRT_USER_H


#include <rtai_lxrt.h>
#include "JR3_lxrt_common.h"


static inline unsigned int JR3DSP_check_sensor_and_DSP( unsigned int dsp )
{
  unsigned int retval;
  struct { unsigned int dsp; } arg = { dsp };
  retval = rtai_lxrt(MYIDX, SIZARG, JR3DSP_CHECK_SENSOR_AND_DSP, &arg).i[LOW];
  return retval;
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
