#ifndef GRIPPER
#define GRIPPER

#include <rtt/TaskContext.hpp>
#include <rtt/TimeService.hpp>

#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>

#include <ocl/dev/IncrementalEncoderSensor.hpp>
#include <ocl/dev/AnalogDrive.hpp>
#include <ocl/dev/Axis.hpp>

#include <ocl/dev/ComediDevice.hpp>
#include <ocl/dev/ComediSubDeviceDOut.hpp>
#include <ocl/dev/ComediSubDeviceAOut.hpp>

using namespace RTT;

namespace OCL{

    class Gripper 
        : public TaskContext
    {
    public:
        Gripper(const std::string& name);
        ~Gripper();
        
    private:
        bool opening,closing,opened,closed;
        
        double pos, prev_pos;
#if (defined OROPKG_OS_LXRT)        
        AnalogOutInterface* ni6713_aout;
        DigitalOutInterface* ni6527_dout;
        EncoderInterface* encoder_if;
        
        DigitalOutput* enable;
        AnalogOutput* voltage;
        AnalogDrive* gripper_driver;
        
        IncrementalEncoderSensor* encoder;
        Axis* gripper_axis;
#endif
        Property<double> voltage_value, eps, min_time;
        Property<bool> actuated_grip;
        
        RTT::TimeService::ticks time_begin;
        RTT::TimeService::Seconds time_passed;

        bool openGripper();
        bool gripperOpened() const;
        
        bool closeGripper();
        bool gripperClosed() const;
        
        /**
         * This function is for the configuration code.
         * Return false to abort configuration.
         */
        bool configureHook();
        
        /**
         * This function is for the application's start up code.
         * Return false to abort start up.
         */
        bool startHook();
        
        /**
         * This function is called by the Execution Engine.
         */
        void updateHook();
        
        /**
         * This function is called when the task is stopped.
         */
        void stopHook();
        
        /**
         * This function is called when the task is being deconfigured.
         */
        void cleanupHook();
    };
    
}//End of Namespace
#endif
