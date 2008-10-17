#ifndef KUKA160_NAXES_VELOCITY_CONTROLLER_HPP
#define KUKA160_NAXES_VELOCITY_CONTROLLER_HPP

#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Command.hpp>
#include <rtt/Properties.hpp>

#if defined (OROPKG_OS_LXRT)
#include "dev/ComediDevice.hpp"
#include "dev/ComediSubDeviceAOut.hpp"
#include "dev/ComediSubDeviceDIn.hpp"
#include "dev/ComediSubDeviceDOut.hpp"
#include "dev/ComediEncoder.hpp"
#include "dev/IncrementalEncoderSensor.hpp"
#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>
#include <rtt/dev/DigitalInput.hpp>
#include "dev/AnalogDrive.hpp"
#include "dev/Axis.hpp"
#endif
#include "dev/SimulationAxis.hpp"
#include <rtt/dev/AxisInterface.hpp>

#include <ocl/OCL.hpp>
#include <ocl/VectorTemplateComposition.hpp>

#include <kdl/chain.hpp>
namespace OCL
{
    /**
     * This class implements a TaskContext to use with the
     * Kuka160 robot in the RoboticLab, PMA, dept. Mechanical
     * Engineering, KULEUVEN. Since the hardware part is very specific
     * for our setup, other people can only use the simulation
     * version. But it can be a good starting point to create your own
     * Robot Software Interface.
     *
     */

    class Kuka160nAxesVelocityController : public RTT::TaskContext
    {
    public:
        /**
         * The contructor of the class.
         *
         * @param name Name of the TaskContext
         * @param propertyfilename name of the propertyfile to
         * configure the component with, default: cpf/Kuka160nAxesVelocityController.cpf
         *
         */

        Kuka160nAxesVelocityController(std::string name);
        virtual ~Kuka160nAxesVelocityController();


    protected:
        /**
         * vector of ReadDataPorts which contain the output velocities
         * of the axes.
         *
         */
        RTT::ReadDataPort<std::vector<double> >   driveValues_port;
        /**
         * vector of WriteDataPorts which contain the values of the
         * reference switch. It can be used for the homing of the robot.
         *
         */
        RTT::DataPort<std::vector<bool> >    referenceValues_port;
        /**
         * vector of DataPorts which contain the values of the
         * position sensors. It is used by other components who need this
         * value for control ;)
         *
         */
        RTT::DataPort<std::vector<double> >       positionValues_port;


        /**
         * The absolute value of the velocity will be limited to this property.
         * Used to fire an event if necessary and to saturate the velocities.
         * It is a good idea to set this property to a low value when using experimental code.
         */
        Property<std::vector<double> > driveLimits_prop;

        /**
         * Lower limit for the positions.  Used to fire an event if necessary.
         */
        Property<std::vector<double> > lowerPositionLimits_prop;

        /**
         * upper limit for the positions.  Used to fire an event if necessary.
         */
        Property<std::vector<double> > upperPositionLimits_prop;

        /**
         *  Start position in rad for simulation.  If the encoders are relative ( like for this component )
         *  also the starting value for the relative encoders.
         */
        Property<std::vector <double> > initialPosition_prop;

        /**
         * Offset to the drive value
         * volt = (setpoint + offset)/scale
         */
        Property<std::vector <double> > driveOffset_prop;

        /**
         * True if simulationAxes should be used in stead of hardware axes
         */
        Property<bool > simulation_prop;
        bool            simulation;

        /**
         * List of Events that should result in an emergencystop
         */
        Property<std::vector<std::string> > EmergencyEvents_prop;


        /**
         * Constant: number of axes
         */
        RTT::Constant<unsigned int> num_axes_attr;

        /**
         * KDL-chain for the Kuka361
         */
        Attribute<KDL::Chain> chain_attr;
        KDL::Chain kinematics;

        /**
         *  parameters to this event is the message that has to be shown.
         *  Each axis that is out of range throws this event.
         *  The component will continue with the previous value.
         */
        RTT::Event< void(std::string) > driveOutOfRange_evt;

        /**
         *  parameters to this event is the message that has to be
         *  shown.  Each axis that is out of range throws this event.
         *  The component will continue.  The hardware limit switches
         *  can be reached when this event is not handled.
         */
        RTT::Event< void(std::string) > positionOutOfRange_evt;

    private:
        //
        // Members implementing the component interface
        //

        //
        // COMMANDS :
        //

        virtual bool startAllAxes();
        virtual bool stopAllAxes();
        virtual bool lockAllAxes();
        virtual bool unlockAllAxes();
        virtual bool addDriveOffset(const std::vector<double>& offset);

        virtual bool initPosition(const std::vector<double>& switchposition);

        virtual bool prepareForUse();
        virtual bool prepareForUseCompleted() const;

        virtual bool prepareForShutdown();
        virtual bool prepareForShutdownCompleted() const;

        void EmergencyStop(std::string message)
        {
            log(Error) << "---------------------------------------------" << endlog();
            log(Error) << "--------- EMERGENCY STOP --------------------" << endlog();
            log(Error) << "---------------------------------------------" << endlog();
            log(Error) << message << endlog();
            this->fatal();
        };
        std::vector<RTT::Handle> EmergencyEventHandlers;

        /**
         * Activation state of robot
         */
        bool activated;

        /**
         * conversion factor between position value and the encoder input.
         * position = (encodervalue)/scale
         */

        std::vector<double>     positionConvertFactor;

        /**
         * conversion factor between drive value and the analog output.
         * volt = (setpoint + offset)/scale
         */

        std::vector<double>     driveConvertFactor;
        std::vector<double>     driveValues,positionValues;
        std::vector<bool>       references;


    public:
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        virtual void cleanupHook();

        //
        // Members implementing the interface to the hardware
        //
#if  (defined (OROPKG_OS_LXRT))
        ComediDevice*                    comediDevAOut;
        ComediDevice*                    comediDevEncoder;
        ComediDevice*                    comediDevDInOut;
        ComediSubDeviceAOut*             comediSubdevAOut;
        ComediSubDeviceDIn*              comediSubdevDIn;
        ComediSubDeviceDOut*             comediSubdevDOut;
        std::vector<RTT::EncoderInterface*>   encoderInterface;

        std::vector<RTT::AnalogOutput*> vref;
        std::vector<OCL::IncrementalEncoderSensor*>   encoder;
        std::vector<RTT::DigitalOutput*>      enable;
        std::vector<OCL::AnalogDrive*>        drive;
        std::vector<RTT::DigitalOutput*>      brake;
        std::vector<RTT::DigitalInput*>       reference;
        std::vector<OCL::Axis*>               axes_hardware;
#endif
        std::vector<OCL::AxisInterface*>      axes;
        std::vector<OCL::SimulationAxis*>     axes_simulation;


    };//class Kuka160nAxesVelocityController
}//namespace Orocos
#endif // KUKA160NAXESVELOCITYCONTROLLER
