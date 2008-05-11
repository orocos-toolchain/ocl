/*
 * Re-used code of comedi/demo directory (v0.8.1)
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "comedi_common.h"

int arm(comedi_t *device, unsigned subdevice, lsampl_t source, unsigned channel)
{
	comedi_insn insn;
	lsampl_t data[2];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_ARM;
	data[1] = source;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

/* This resets the count to zero and disarms the counter.  The counter output
 is set low. */
int reset_counter(comedi_t *device, unsigned subdevice, unsigned channel)
{
	comedi_insn insn;
	lsampl_t data[1];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_RESET;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int set_counter_mode(comedi_t *device, unsigned subdevice, lsampl_t mode_bits, unsigned channel)
{
	comedi_insn insn;
	lsampl_t data[2];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_COUNTER_MODE;
	data[1] = mode_bits;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int set_clock_source(comedi_t *device, unsigned subdevice, lsampl_t clock, lsampl_t period_ns, unsigned channel)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_CLOCK_SRC;
	data[1] = clock;
	data[2] = period_ns;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int set_gate_source(comedi_t *device, unsigned subdevice, lsampl_t gate_index, lsampl_t gate_source, unsigned channel)
{
	comedi_insn insn;
	lsampl_t data[3];
	int retval;

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_CONFIG;
	insn.subdev = subdevice;
	insn.chanspec = channel;
	insn.data = data;
	insn.n = sizeof(data) / sizeof(data[0]);
	data[0] = INSN_CONFIG_SET_GATE_SRC;
	data[1] = gate_index;
	data[2] = gate_source;

	retval = comedi_do_insn(device, &insn);
	if(retval < 0)
	{
		fprintf(stderr, "%s: error:\n", __FUNCTION__);
		comedi_perror("comedi_do_insn");
		return retval;
	}
	return 0;
}

int comedi_internal_trigger(comedi_t *dev, unsigned int subd, unsigned int trignum)
{
	comedi_insn insn;
	lsampl_t data[1];

	memset(&insn, 0, sizeof(comedi_insn));
	insn.insn = INSN_INTTRIG;
	insn.subdev = subd;
	insn.data = data;
	insn.n = 1;

	data[0] = trignum;

	return comedi_do_insn(dev, &insn);
}
