#ifndef KUKA361_NAXES_VELOCITY_CONTROLLER_HPP
#define KUKA361_NAXES_VELOCITY_CONTROLLER_HPP

#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>

#include "VectorTemplateComposition.hpp"

#if defined (OROPKG_OS_LXRT)
#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>

#include "dev/SwitchDigitalInapci1032.hpp"
#include "dev/RelayCardapci2200.hpp"
#include "dev/EncoderSSIapci1710.hpp"
#include "dev/ComediDevice.hpp"
#include "dev/ComediSubDeviceAOut.hpp"

#include "dev/AbsoluteEncoderSensor.hpp"
#include "dev/AnalogDrive.hpp"
#include "dev/Axis.hpp"
#endif
#include "dev/SimulationAxis.hpp"
#include <rtt/dev/AxisInterface.hpp>

#include <ocl/OCL.hpp>

#include <kdl/chain.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext to use with the
     * Kuka361 robot in the RoboticLab, PMA, dept. Mechanical
     * Engineering, KULEUVEN. Since the hardware part is very specific
     * for our setup, other people can only use the simulation
     * version. But it can be a good starting point to create your own
     * Robot Software Interface.
     * 
     */
    
    class Kuka361nAxesVelocityController : public TaskContext
    {
    public:
        /** 
         * The contructor of the class.
         * 
         * @param name Name of the TaskContext
         * configure the component with, default: cpf/Kuka361nAxesVelocityController.cpf
         * 
         */
        Kuka361nAxesVelocityController(std::string name);
        virtual ~Kuka361nAxesVelocityController();
    
    protected:  
        /** 
         * Method to start all axes .
         *
         * Sets the axis in the DRIVEN state. Only possible if the axis
         * is int the STOPPED state. If succesfull the drive value of
         * the axis is setted to zero and will be updated periodically
         * 
         * @return Can only succeed if all axis are in the DRIVEN state
         */
        Method<bool(void)> startAllAxes_mtd; 
        
        /**
         * Method to stop all axes .
         *
         * Sets the drive value to zero and changes to the STOP
         * state. Only possible if axis is in the DRIVEN state. In the
         * stop state, the axis does not listen and write to its 
         * ReadDataPort _driveValue.
         *
         * @return false if in wrong state or already stopped.
         */
        Method<bool(void)> stopAllAxes_mtd; 
        
        /** 
         * Method to unlock all axes .
         *
         * Activates the brake of the axis.  Only possible in the STOPPED state.
         * 
         * @return false if in wrong state or already locked.
         */
        Method<bool(void)> unlockAllAxes_mtd;
        
        /** 
         * Method to lock all axes.
         *
         * Releases the brake of the axis.  Only possible in the LOCKED
         * state.
         * 
         * @return false if in wrong state or already locked.
         */
        Method<bool(void)> lockAllAxes_mtd; 

        /**
         * Method to prepare robot for use.
         *
         * It is needed to activate the hardware controller of the
         * robot. 
         * 
         * @return Will only be true if the hardware controller is ready
         * and the emergency stops are released. 
         *
         */
        Command<bool(void)> prepareForUse_cmd; 
        
        /** 
         * Command to Shutdown the hardware controller of the robot.
         * 
         */
        Command<bool(void)> prepareForShutdown_cmd;
        
        /**
         * Method to add drive offsets to the axes.
         *
         * Adds an offset to the drivevalues of the axes and updates the
         * driveOffset values.
         * 
         * @param offset offset value in geometrics units [rad/s]
         * 
         */
        Method<bool(std::vector<double>)> addDriveOffset_mtd;

        /**
         * DataPort which contain the output velocities
         * of the axes.  
         * 
         */
        DataPort<std::vector<double> > driveValues_port;

        /**
         * DataPort which contain the values of the
         * position sensors. It is used by other components who need this
         * value for control ;)
         * 
         */
        DataPort<std::vector<double> >  positionValues_port;

        /**
         * The absolute value of the velocity will be limited to this property.  
         * Used to fire an event if necessary and to saturate the velocities.
         * It is a good idea to set this property to a low value when using experimental code.
         */
        Property<std::vector <double> > driveLimits_prop;
        
        /**
         * Lower limit for the positions.  Used to fire an event if necessary.
         */
        Property<std::vector <double> > lowerPositionLimits_prop;
        
        /**
         * upper limit for the positions.  Used to fire an event if necessary.
         */
        Property<std::vector <double> > upperPositionLimits_prop;
        
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
         * True if geometric axes values should be used in stead of
         * actuator values.
         */
        Property<bool > geometric_prop;

        /** 
         * List of Events that should result in an emergencystop
         */
        Property<std::vector<std::string> > EmergencyEvents_prop;
                
        /**
         * Constant Attribute: number of axes
         */
        Constant<unsigned int> num_axes_attr;

        /**
         * KDL-chain for the Kuka361
         */
        Attribute<KDL::Chain> chain_attr;
        KDL::Chain kinematics;
                
        /**
         *  parameters to this event are the axis and the velocity that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue with the previous value.
         */
        Event< void(std::string) > driveOutOfRange_evt;
        
        /**
         *  parameters to this event are the axis and the position that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue.  The hardware limit switches can be reached when this
         *  event is not handled.
         */ 
        Event< void(std::string) > positionOutOfRange_evt;

      

    private:    

        virtual bool startAllAxes();
        virtual bool stopAllAxes();
        virtual bool lockAllAxes();
        virtual bool unlockAllAxes();
        virtual bool addDriveOffset(const std::vector<double>& offset);
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
        
        std::vector<double> positionConvertFactor;
    
    
        /**
         * conversion factor between drive value and the analog output.
         * volt = (setpoint + offset)/scale
         */
        
        std::vector<double> driveConvertFactor;


        ///Local copy for the position and drive values:
        std::vector<double> positionValues, driveValues,
            positionValues_kin,driveValues_rob;
        
    public:
        
        virtual bool configureHook();
        virtual bool startHook(); 
        virtual void updateHook();
        virtual void stopHook();
        virtual void cleanupHook();

        KDL::Chain getKinematics(){
            return kinematics;
        };
        
    private:
        
        void convertGeometric();
              
        // 
        // Members implementing the interface to the hardware
        //
#if  (defined (OROPKG_OS_LXRT))
        std::vector<Axis*>  axes_hardware;
        
        ComediDevice*                    comediDev;
        ComediSubDeviceAOut*             comediSubdevAOut;
        EncoderSSI_apci1710_board*       apci1710;
        RelayCardapci2200*               apci2200;
        SwitchDigitalInapci1032*         apci1032;

        std::vector<EncoderInterface*>           encoderInterface;
        std::vector<AbsoluteEncoderSensor*>      encoder;
        std::vector<AnalogOutput<unsigned int>*> vref;
        std::vector<DigitalOutput*>              enable;
        std::vector<AnalogDrive*>                drive;
        std::vector<DigitalOutput*>              brake;
    
#endif
        std::vector<AxisInterface*>      axes;
    
    };//class Kuka361nAxesVelocityController

}//namespace Orocos
#endif // KUKA361NAXESVELOCITYCONTROLLER
