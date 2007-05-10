#ifndef ANALOGDETHERCATINPUTDEVICE_HPP
#define ANALOGDETHERCATOUTPUTDEVICE_HPP

#include "rtt/dev/AnalogInInterface.hpp"

namespace RTT
{
    /**
	 * A test class which replaces a real device driver.
	 * It reproduces on the output what it gets on the input.
	  */
    struct AnalogEtherCATInputDevice :
			 public AnalogInInterface<unsigned int>
			 {
				 unsigned char* mstartaddress;
				 unsigned int nbofchans;
				 unsigned int mbin_range;
				 double mlowest, mhighest;

				 AnalogEtherCATInputDevice(unsigned char* startaddress, unsigned int channels=2, unsigned int bin_range=4096, double lowest = -5.0, double highest = +5.0)
					 : AnalogInInterface<unsigned int>("AnalogInDevice"),
				 mstartaddress(startaddress),
				 nbofchans(channels),
				 mbin_range( bin_range),
				 mlowest( lowest),
				 mhighest( highest)
				 {}

				 ~AnalogEtherCATInputDevice() {}

				 virtual void rangeSet(unsigned int /*chan*/, 
											  unsigned int /*range*/) {}

				 virtual void arefSet(unsigned int /*chan*/, 
											 unsigned int /*aref*/) {}

				 virtual unsigned int nbOfChannels() const {
					 return nbofchans;
				 }

				 virtual void read( unsigned int chan, unsigned int& value ) const {
					 if (chan < nbofchans) {
						 unsigned int tmp = mstartaddress[(chan * 3) + 2];
						 tmp = tmp<<8;
						 tmp = tmp | mstartaddress[(chan * 3) + 1];
						 value = tmp;
					 }
				 }
		  
				 virtual void read( unsigned int chan, double& value ) const {
					 if (chan < nbofchans) {
						 unsigned int tmp;
						 read(chan, tmp);
						 value = 1.0 * tmp * (mhighest - mlowest) / mbin_range + mlowest;
					 }
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
