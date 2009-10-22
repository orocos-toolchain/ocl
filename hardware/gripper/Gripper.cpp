#include "Gripper.hpp"
#include <rtt/Logger.hpp>
#include <rtt/Command.hpp>
#include <ocl/ComponentLoader.hpp>

#include "../staubli/ComediDevices.hpp"

#include <math.h>

ORO_CREATE_COMPONENT( OCL::Gripper )

namespace OCL{

    Gripper::Gripper(const std::string& name) :
        TaskContext(name),
        opening(false),closing(false),opened(false),closed(false),
        voltage_value("voltage","",9.9),
        eps("eps","",0.01),
        min_time("min_time","",1.0),
        actuated_grip("actuated_grip","",true)
    {
        this->commands()->addCommand(command("open", &Gripper::openGripper, &Gripper::gripperOpened, this), "Opening gripper.");
        this->commands()->addCommand(command("close", &Gripper::closeGripper, &Gripper::gripperClosed, this), "Closing gripper.");

        this->properties()->addProperty(&voltage_value);
        this->properties()->addProperty(&eps);
        this->properties()->addProperty(&min_time);
        this->properties()->addProperty(&actuated_grip);
#if (defined OROPKG_OS_LXRT)
        ComediLoader::Instance()->CreateComediDevices();
        //check if the devices already exist:
        ni6527_dout = DigitalOutInterface::nameserver.getObject("DigitalOut");
        ni6713_aout = AnalogOutInterface::nameserver.getObject("AnalogOut");
        encoder_if = EncoderInterface::nameserver.getObject("Counter6");

        encoder = new IncrementalEncoderSensor(encoder_if,1.0,0.0,0.0,0.0,1);
        enable = new DigitalOutput(ni6527_dout,0);
        voltage = new AnalogOutput(ni6713_aout,6);
        gripper_driver = new AnalogDrive(voltage,enable);
        gripper_axis = new Axis(gripper_driver);
#endif
    }

    bool Gripper::configureHook(){
        return true;
    }

    Gripper::~Gripper()
    {
#if (defined OROPKG_OS_LXRT)
        delete gripper_axis;
        delete encoder;
        ComediLoader::Instance()->DestroyComediDevices();
#endif
    }

    bool Gripper::openGripper()
    {
        opening=true;
        closing=false;
        opened=false;
        time_begin=TimeService::Instance()->getTicks();
        return true;
    }

    bool Gripper::closeGripper()
    {
        closing=true;
        opening=false;
        closed=false;
        time_begin=TimeService::Instance()->getTicks();
        return true;
    }

    bool Gripper::gripperOpened() const
    {
        return opened;
    }

    bool Gripper::gripperClosed() const
    {
        return closed;
    }

    bool Gripper::startHook()
    {
        bool ok=true;
#if (defined OROPKG_OS_LXRT)
        ok &= gripper_axis->unlock();
        ok &= gripper_axis->drive(0.0);
#endif
        return ok;
    }

    void Gripper::updateHook()
    {
#if (defined OROPKG_OS_LXRT)

        prev_pos=pos;
        pos=encoder->readSensor();
        bool stopped = false;
        time_passed = TimeService::Instance()->secondsSince(time_begin);

        if(time_passed>min_time.rvalue())
            stopped = (std::abs(prev_pos-pos)/getPeriod())<eps.rvalue();
#endif
        if(opening){
#if (defined OROPKG_OS_LXRT)
            gripper_axis->drive(voltage_value.rvalue());
            if(stopped)
#endif
                {
                    opening=false;
                    opened=true;
                }
        }
        else if(closing){
#if (defined OROPKG_OS_LXRT)
            gripper_axis->drive(-voltage_value.rvalue());
            if(stopped)
#endif
                {
                    closed=true;
                    if(!actuated_grip.rvalue())
                        closing=false;
                }
        }else{
#if (defined OROPKG_OS_LXRT)
            gripper_axis->drive(0.0);
#endif
        }
    }

    void Gripper::stopHook()
    {
#if (defined OROPKG_OS_LXRT)
        gripper_axis->stop();
        gripper_axis->lock();
#endif
    }

    void Gripper::cleanupHook()
    {

    }

}
