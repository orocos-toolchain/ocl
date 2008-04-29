#ifndef PERFORMERMK2_NAXES_VELOCITY_CONTROLLER_HPP
#define PERFORMERMK2_NAXES_VELOCITY_CONTROLLER_HPP

#include <vector>
#include <deque>
#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>

#if defined (OROPKG_OS_LXRT)
#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>
#include <rtt/dev/DigitalInput.hpp>

#include "dev/ComediDevice.hpp"
#include "dev/ComediSubDeviceAOut.hpp"
#include "dev/ComediSubDeviceDOut.hpp"
#include "dev/ComediSubDeviceDIn.hpp"
#include "dev/ComediEncoder.hpp"

#include "dev/IncrementalEncoderSensor.hpp"
#include "dev/AnalogDrive.hpp"
#include "dev/Axis.hpp"

#include <termios.h>

#endif
#include "dev/SimulationAxis.hpp"
#include <rtt/dev/AxisInterface.hpp>

#include <ocl/OCL.hpp>

#include <kdl/chain.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext to use with the
     * PerformerMK2 robot in the RoboticLab, PMA, dept. Mechanical
     * Engineering, KULEUVEN. Since the hardware part is very specific
     * for our setup, other people can only use the simulation
     * version. But it can be a good starting point to create your own
     * Robot Software Interface.
     * 
     */
    
    class PerformerMK2nAxesVelocityController : public TaskContext
    {
    public:
        /** 
         * The contructor of the class.
         * 
         * @param name Name of the TaskContext
         * @param propertyfilename name of the propertyfile to
         * configure the component with, default: cpf/PerformerMK2nAxesVelocityController.cpf
         * 
         */
        PerformerMK2nAxesVelocityController(std::string name,std::string propertyfilename="cpf/PerformerMK2nAxesVelocityController.cpf");
        virtual ~PerformerMK2nAxesVelocityController();
    
    protected: 


        Method<bool(int)> resetController_mtd;
        /** 
         * method to reset the velocity Controller.
         *
         */
 
        /** 
         * method to start all axes .
         *
         * sets the axis in the driven state. only possible if the axis
         * is int the stopped state. if succesfull the drive value of
         * the axis is setted to zero and will be updated periodically
         * 
         * @return can only succeed if all axis are in the driven state
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
       * Method to set the position of an axis to its initial
       * position.
       *
       * Sets the PerformernAxesVelocityController::_positionValue to
       * the PerformernAxesVelocityController::_initialPosition from the 
       * property-file. This is needed because the Performer needs to be homed.
       * 
       * @param axis nr of axis
       */
	Method<bool(int)>  _initPosition;

        /**
         * DataPort which contain the output velocities
         * of the axes.  
         * 
         */
        DataPort<std::vector<double> > driveValues_port;
        DataPort<std::vector<double> > servoValues_port;

        /**
         * DataPort which contain the values of the
         * position sensors. It is used by other components who need this
         * value for control ;)
         * 
         */
        DataPort<std::vector<double> >  positionValues_port;
        DataPort<std::vector<double> >  velocityValues_port;
        DataPort<std::vector<double> >  jValues_port;
        DataPort<std::vector<double> >  homingSwitchValues_port;
        DataPort< double >              deltaTime_port;

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
         * limits for the velocities.  Used to fire an event if necessary.
         */
        Property<std::vector <double> > velocityLimits_prop;
        
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
        
        Property<std::vector<double> > servoIntegrationFactor_prop;
        Property<std::vector<double> > servoGain_prop;
        Property<std::vector<double> > servoFFScale_prop;
    	Property<std::vector<double> > PIDkp_prop;
        Property<std::vector<double> > PIDTi_prop;
	Property<std::vector<double> > PIDTd_prop;

        /**
         * Constant Attribute: number of axes
         */
        Constant<unsigned int> num_axes_attr;

        /**
         * KDL-chain for the PerformerMK2
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
        Event< void(std::string) > velocityOutOfRange_evt;
        
    private:    

        virtual bool resetController(const int& axis_id);
        virtual bool startAllAxes();
        virtual bool stopAllAxes();
        virtual bool lockAllAxes();
        virtual bool unlockAllAxes();
        virtual bool addDriveOffset(const std::vector<double>& offset);
	virtual bool initPosition(int axis);
      	virtual bool initPositionCompleted(int axis) const;
        virtual bool prepareForUse();
        virtual bool prepareForUseCompleted() const;
        virtual bool prepareForShutdown();
        virtual bool prepareForShutdownCompleted() const;
      
        /**
         * A local copy of the name of the propertyfile so we can store
         * changed properties.
         */
        const std::string propertyfile;
    
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
        std::vector<double> positionValues, driveValues, velocityValues, jValues;
	std::deque<double> positionDeque_axis1, positionDeque_axis2, positionDeque_axis3, positionDeque_axis4, positionDeque_axis5, timeDeque;
        std::vector<double>   homingSwitchValues;
        ///local copy for the servoloop parameters
        std::vector<double> servoIntError,outputvel;
        std::vector<double> servoIntegrationFactor,servoGain,servoFFScale, PIDkp, PIDTi, PIDTd;
        std::vector<double> outputvel_kmin1,outputvel_kmin2,velocity_error_k,velocity_error_kmin1,velocity_error_kmin2,position_error_k,position_desired_k;
	bool servoInitialized;
        RTT::TimeService::ticks    previousTime;

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
        
        // 
        // Members implementing the interface to the hardware
        //
#if  (defined (OROPKG_OS_LXRT))
        std::vector<Axis*>  axes_hardware;
        
        ComediDevice*                    NI6713;
        ComediDevice*                    NI6602;
        
        ComediSubDeviceAOut*             SubAOut_NI6713;
        ComediSubDeviceDIn*              SubDIn_NI6713;
        ComediSubDeviceDOut*             SubDOut_NI6602;
        
        std::vector<EncoderInterface*>           encoderInterface;
        std::vector<IncrementalEncoderSensor*>   encoder;
        std::vector<AnalogOutput*> vref;
        std::vector<DigitalOutput*>              enable;
        std::vector<AnalogDrive*>                drive;
        DigitalOutput*                           brakeAxis2;
        DigitalOutput*                           brakeAxis3;
        DigitalInput*                            armPowerOn;
        DigitalOutput*                           armPowerEnable;
        std::vector<DigitalInput*>               homingSwitch;

#endif
        std::vector<AxisInterface*>              axes;
	TimeService::ticks                       previous_time, initial_time;
	TimeService::Seconds                     delta_time, total_time;
    
    };//class PerformerMK2nAxesVelocityController
}//namespace Orocos
#endif // PERFORMERNAXESVELOCITYCONTROLLER
