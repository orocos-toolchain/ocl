#ifndef ETHERCATENCODER_HPP
#define ETHERCATENCODER_HPP

#include "rtt/dev/EncoderInterface.hpp"
#include <rtt/DataObjectInterfaces.hpp>

namespace RTT
{
    /**
	 * @brief An EtherCAT Encoder (EL5101)
	  */
	class EtherCATEncoder
	: public EncoderInterface
	{
		struct SetPosInfo {
			unsigned int setpos;
			bool setposreq;
		};
		struct PosInfo {
			int pos;
			bool upcounting;
			int turn;
		};
		DataObjectLockFree<PosInfo> mPosLF;
		DataObjectLockFree<SetPosInfo> mSetPosInfoLF;
		unsigned int mresolution;
		unsigned char* mstartaddressin;
		unsigned char* mstartaddressout;
		unsigned char prevoverfl;		// The previous state of the overflow bit
		unsigned char prevunderfl;		// The previous state of the underflow bit
		public:
			
			
			
			EtherCATEncoder(unsigned char* startaddressinput, unsigned char* startaddressoutput, unsigned int resolution = 65536, bool upcounting = true)
			: mPosLF( "Pos" ),
			mSetPosInfoLF( "PosInfo" ),
			mresolution(resolution), 
			mstartaddressin(startaddressinput),
			mstartaddressout(startaddressoutput),
			prevoverfl(0),
			prevunderfl(0)
			{
				// Initialise Info for setting a position
				SetPosInfo setposinit;
				setposinit.setpos = 0; setposinit.setposreq = false;
				mSetPosInfoLF.Set(setposinit);
				
				// Initialise position info
				PosInfo	posinit;
				posinit.pos = 0; posinit.upcounting = upcounting; posinit.turn = 0;
				mPosLF.Set(posinit);
			}
			virtual ~EtherCATEncoder() {}

			void update()
			{
				PosInfo postmp;
				SetPosInfo setpostmp;
				mPosLF.Get( postmp );
				mSetPosInfoLF.Get( setpostmp );
				
				int prevpos = postmp.pos;
				int prevturn = postmp.turn;
				
				// Update the current turn
				unsigned char status = mstartaddressin[0];	//Read the status byte
				if( (prevoverfl == 0) && ((status>>4 & 1) == 1) )	// Increment turn, when an overflow occurse, overflow bit is the 4th bit
					postmp.turn++;
				else if( (prevunderfl == 0) && ((status>>3 & 1) == 1) )	// Decrement turn, when an underflow occurse, underflow bit is the 3rd bit
					postmp.turn--;
				prevoverfl = status>>4 & 1;
				prevunderfl = status>>3 & 1;
				
				if(!setpostmp.setposreq) {
					mstartaddressout[0] = mstartaddressout[0] & 0xfb;	// Clear the CNT_SET bit in Control byte
					
					// Update the position
					postmp.pos = mstartaddressin[2];	// copy counter value 
					postmp.pos = postmp.pos<<8;
					postmp.pos = postmp.pos | mstartaddressin[1];
				}
				else {	// There is a request to set the position
					mstartaddressout[1] = setpostmp.setpos;	// Copy the requested position
					mstartaddressout[2] = setpostmp.setpos>>8;
					mstartaddressout[0] = mstartaddressout[0] | 0x04;	// Set the CNT_SET bit in Control byte
					setpostmp.setposreq = false;
				}
				
				// Check the counting direction
				if(prevturn > postmp.turn)
					postmp.upcounting = false;
				else if(prevturn < postmp.turn)
					postmp.upcounting = true;
				else if(prevpos > postmp.pos)
					postmp.upcounting = false;
				else if(prevpos < postmp.pos)
					postmp.upcounting = true;
				
				mPosLF.Set( postmp );
				mSetPosInfoLF.Set( setpostmp );
			}
			
        /**
			 * Get the position within the current turn.
			*/
			virtual int positionGet() const { 
				return mPosLF.Get().pos;
			}

        /**
			 * Get the current turn.
			*/
			virtual int turnGet() const {
				return mPosLF.Get().turn;
			}
           
        /**
			 * Set the position within the current turn.
			*/
			virtual void positionSet( int pos) { 
				SetPosInfo settmp;
				settmp.setpos = pos;
				settmp.setposreq = true;
				mSetPosInfoLF.Set(settmp);
			}

        /**
			 * Set the current turn.
			*/
			virtual void turnSet( int t ) { 
				PosInfo tmp;
				mPosLF.Get(tmp);
				tmp.turn = t;
				mPosLF.Set(tmp);
			}

        /**
			 * Return the position resolution. This number
			 * can be negative or positive and denotes the
			 * the maximal or minimal value positionGet().
			*/
			virtual int resolution() const { return mresolution; }

        /**
			 * Returns true if after a positive turn increment,
			 * position increments positively.
			 * Meaning from 0 to |resolution()| or from
			 * resolution() to zero if resolution() < 0
			*/
			virtual bool upcounting() const { 
				return mPosLF.Get().upcounting; 
			}
	};
}

#endif
