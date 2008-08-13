#ifndef ETHERCAT_DEMO_IO_HPP
#define ETHERCAT_DEMO_IO_HPP

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
    class EthercatDemoIO : public RTT::TaskContext
    {

	Property<std::string> rteth;
	Method <bool(double)> gen_sin;
	bool mgen_sin(double);
	Method <void(void)> stop_master;
	void mstop_master(void);
	struct netif *ni;
	int cnt, cnt_dig;
	int voltage_i;
	float voltage_f, time, sinus_freq;



    public:
   		EthercatDemoIO(std::string name);
			~EthercatDemoIO();

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
