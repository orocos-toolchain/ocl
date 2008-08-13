
#ifndef _COMEDI_COMMON_H
#define _COMEDI_COMMON_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* some helper functions used primarily for counter demos */
extern int arm(comedi_t *device, unsigned subdevice, lsampl_t source);
extern int reset_counter(comedi_t *device, unsigned subdevice);
extern int set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits);
extern int set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns);
extern int set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source);
extern int comedi_internal_trigger(comedi_t *dev, unsigned int subd, unsigned int trignum);

#ifdef __cplusplus
}
#endif

#define sec_to_nsec(x) ((x)*1000000000)
#define sec_to_usec(x) ((x)*1000000)
#define sec_to_msec(x) ((x)*1000)
#define msec_to_nsec(x) ((x)*1000000)
#define msec_to_usec(x) ((x)*1000)
#define usec_to_nsec(x) ((x)*1000)

#endif

