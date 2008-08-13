#ifndef DIGITALETHERCATOUTPUTDEVICE_HPP
#define DIGITALETHERCATOUTPUTDEVICE_HPP

#include "rtt/dev/DigitalOutInterface.hpp"
#include <vector>

namespace RTT
{
    /**
	 * A Fake (Simulated) Digital Input/Output Device which replicates the inputs
	 * on its outputs.
	  */
	class DigitalEtherCATOutputDevice
		:	public DigitalOutInterface
	{
		public:
			std::vector<bool> mchannels;
			unsigned char* mstartaddress;
			unsigned int startb;
			unsigned int nbofchannels;

			DigitalEtherCATOutputDevice(unsigned char* startaddress, unsigned int start_bit,unsigned int channels=32)
			: 		DigitalOutInterface("DigitalEtherCATOutputDevice"),
					mchannels(channels,false),
					mstartaddress(startaddress),
					startb(start_bit),
					nbofchannels(channels)
			{}

			virtual void switchOn( unsigned int n )
			{
				if ( n < nbofchannels ) {
					unsigned char tmp = 1 << n+startb;
					*mstartaddress = *mstartaddress | tmp;
				}
			}

			virtual void switchOff( unsigned int n )
			{
				if ( n < nbofchannels ) {
					unsigned char tmp = (1 << n+startb) ^ 0xff;
					*mstartaddress = *mstartaddress & tmp;
				}
			}

			virtual void setBit( unsigned int bit, bool value )
			{
				if(value)
					switchOn(bit);
				else
					switchOff(bit);
			}

			virtual void setSequence(unsigned int start_bit, unsigned int stop_bit, unsigned int value)
			{
				if ( start_bit < nbofchannels && stop_bit < nbofchannels ) {
					//unsigned int tmp = 0xffffffff << startb + nbofchannels;
					for (unsigned int i = start_bit; i <= stop_bit; i++)
						mchannels[i] = value & ( 1<<( i - start_bit ) );
				}

			}

			virtual bool checkBit(unsigned int n) const
			{
				if ( n < nbofchannels )
					return mchannels[n];
				return false;
			}


			virtual unsigned int checkSequence( unsigned int start_bit, unsigned int stop_bit ) const
			{
				unsigned int result = 0;
				if ( start_bit < nbofchannels && stop_bit < nbofchannels )
					for (unsigned int i = start_bit; i <= stop_bit; ++i)
						result += (mchannels[i] & 1)<<i;
				return result;
			}

			virtual unsigned int nbOfOutputs() const
			{
				return nbofchannels;
			}

	};


}


#endif
