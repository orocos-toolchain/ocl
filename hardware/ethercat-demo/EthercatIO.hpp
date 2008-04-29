#ifndef ETHERCAT_IO_HPP
#define ETHERCAT_IO_HPP

#include "dev/DigitalEtherCATOutputDevice.hpp"
#include "dev/DigitalEtherCATInputDevice.hpp"
#include "dev/AnalogEtherCATOutputDevice.hpp"
#include "dev/AnalogEtherCATInputDevice.hpp"
#include "dev/EtherCATEncoder.hpp"

#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>

#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>

#include <ocl/OCL.hpp>

struct netif;

namespace OCL
{
    /**
     * 
     */
    class EthercatIO : public RTT::TaskContext
    {

	Property<std::string> rteth;
	Method <int(void)> get_pos;
	Method <void(int)> set_pos;
	Method <int(void)> get_turn;
	Method <void(int)> set_turn;
	Method <bool(void)> upcounting;
	int mget_pos(void);
	void mset_pos(int);
	int mget_turn(void);
	void mset_turn(int);
	bool mupcounting(void);
	struct netif *ni;
	int cnt_dig;
	unsigned char* fmmu_buffer;
	DigitalEtherCATOutputDevice digoutputs;
	DigitalEtherCATOutputDevice digoutputs2;
	DigitalEtherCATInputDevice diginputs;
	AnalogEtherCATOutputDevice anaoutputs;
	AnalogEtherCATInputDevice anainputs;
	EtherCATEncoder enc;
	double prevvoltage;
	//bool ones;
	
    
    public:
   		EthercatIO(std::string name);
			~EthercatIO();
			
			DigitalInInterface* getDigitalIn() { return &diginputs;}
			DigitalOutInterface* getDigitalOut() { return &digoutputs;}
			AnalogInInterface* getAnalogIn() { return &anainputs;}
			AnalogOutInterface* getAnalogOut() { return &anaoutputs;}

	     /**
         *  This function contains the application's startup code.
         *  Return false to abort startup.
         **/
        virtual bool startup(); 
        
        /**
         * This function is periodically called.
         */
        virtual void update();
        
        /**
         * This function is called when the task is stopped.
         */
        virtual void shutdown();
    
    };//class EthercatDemoIO
}//namespace Orocos
#endif // EthercatDemoIO
