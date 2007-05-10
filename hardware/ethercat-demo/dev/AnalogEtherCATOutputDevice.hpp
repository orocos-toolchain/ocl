#ifndef ANALOGETHERCATOUTPUTDEVICE_HPP
#define ANALOGETHERCATOUTPUTDEVICE_HPP

#include "rtt/dev/AnalogOutInterface.hpp"

namespace RTT
{
    /**
	 * A test class which replaces a real device driver.
	 * It reproduces on the output what it gets on the input.
	  */
    struct AnalogEtherCATOutputDevice :
			 public AnalogOutInterface<unsigned int>
			 {
				 unsigned char* mstartaddress;
				 unsigned int nbofchans;
				 double* mchannels;
				 unsigned int mbin_range;
				 double mlowest, mhighest;

				 AnalogEtherCATOutputDevice(unsigned char* startaddress, unsigned int channels=2, unsigned int bin_range=32676, double lowest = 0, double highest = 10)
					 : AnalogOutInterface<unsigned int>("AnalogOutDevice"),
				 mstartaddress(startaddress),
				 nbofchans(channels),
				 mchannels( new double[channels] ),
				 mbin_range( bin_range),
				 mlowest( lowest),
				 mhighest( highest)
				 {}

				 ~AnalogEtherCATOutputDevice() {
					 delete[] mchannels;
				 }

				 virtual void rangeSet(unsigned int /*chan*/, 
											  unsigned int /*range*/) {}

				 virtual void arefSet(unsigned int /*chan*/, 
											 unsigned int /*aref*/) {}

				 virtual unsigned int nbOfChannels() const {
					 return nbofchans;
				 }

				 virtual void write( unsigned int chan, unsigned int value ) {
					 if (chan < nbofchans && value >= 0 && value <= mbin_range) {
						 unsigned int tmp = value;
						 mstartaddress[(chan * 2)] = tmp;
						 mstartaddress[(chan * 2) + 1]= (tmp>>8);
						 mchannels[chan] = value;
					 }
				 }
				 
				 void write( unsigned int chan, double value ) {
					 unsigned int tmp = (unsigned int)((value - mlowest) * mbin_range / (mhighest - mlowest)) ;
					 write(chan, tmp);
				 }

				 virtual unsigned int binaryRange() const
				 {
					 return mbin_range;
				 }

				 virtual unsigned int binaryLowest() const 
				 {
					 return 0;
				 }

				 virtual unsigned int binaryHighest() const
				 {
					 return mbin_range;
				 }

				 virtual double lowest(unsigned int /*chan*/) const
				 {
					 return mlowest;
				 }

				 virtual double highest(unsigned int /*chan*/) const
				 {
					 return mhighest;
				 }

				 virtual double resolution(unsigned int /*chan*/) const
				 {
					 return mbin_range/(mhighest-mlowest);
				 }

			 };    

}

#endif
