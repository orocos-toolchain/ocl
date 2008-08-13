#ifndef DIGITALETHERCATINPUTDEVICE_HPP
#define DIGITALETHERCATINPUTDEVICE_HPP

#include "rtt/dev/DigitalInInterface.hpp"
#include <vector>

namespace RTT
{
    /**
	 * A Digital Input Device
	  */
	class DigitalEtherCATInputDevice
	: public DigitalInInterface
	{
		public:
			std::vector<bool> mchannels;
			unsigned int nbofchannels;
			unsigned int startb;
			unsigned char* mstartaddress;

			DigitalEtherCATInputDevice(unsigned char* startaddress, unsigned int start_bit, unsigned int channels=32)
			: DigitalInInterface("DigitalEtherCATInputDevice"),
			mchannels(channels,false),
			nbofchannels(channels),
			startb(start_bit),
			mstartaddress(startaddress)
			{}


			virtual unsigned int nbOfInputs() const
			{
				return nbofchannels;
			}

			virtual bool isOn( unsigned int bit = 0) const
			{
				if ( bit < mchannels.size() )
					return readBit(0);
				return false;
			}

			virtual bool isOff( unsigned int bit = 0) const
			{
				if ( bit < mchannels.size() )
					return !readBit(0);
				return true;
			}

			virtual bool readBit( unsigned int bit = 0) const
			{
				if ( bit < nbofchannels ) {
					unsigned char tmp = *mstartaddress;
					tmp = tmp >> startb + bit;
					return tmp & 0x01;
				}
				return false;
			}

			virtual unsigned int readSequence(unsigned int start_bit, unsigned int stop_bit) const
			{
				if ( start_bit < mchannels.size() && stop_bit < mchannels.size() )
					return 0;
				return 0;
			}

	};


}


#endif
