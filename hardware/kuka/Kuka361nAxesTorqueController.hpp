#ifndef KUKA361_NAXES_TORQUE_CONTROLLER_HPP
#define KUKA361_NAXES_TORQUE_CONTROLLER_HPP

#include <rtt/RTT.hpp> 

#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

#if (defined (OROPKG_OS_LXRT))
#include "dev/SwitchDigitalInapci1032.hpp"
#include "dev/RelayCardapci2200.hpp"
#include "dev/EncoderSSIapci1710.hpp"
#include "dev/ComediDevice.hpp"
#include "dev/ComediSubDeviceAOut.hpp"
#include "dev/ComediSubDeviceAIn.hpp"
#include "dev/ComediSubDeviceDOut.hpp"
#include "dev/ComediSubDeviceDIn.hpp"
#include "dev/AbsoluteEncoderSensor.hpp"
#include "dev/AnalogDrive.hpp"
#include "dev/AnalogSensor.hpp"
#include "dev/Axis.hpp"

#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/AnalogInput.hpp>
#include <rtt/dev/DigitalOutput.hpp>
#include <rtt/dev/DigitalInput.hpp>


#endif

#include "dev/SimulationAxis.hpp"
#include "dev/TorqueSimulationAxis.hpp"
#include <rtt/dev/AxisInterface.hpp>
#include "Kuka361TorqueSimulator.hpp"

#include <ocl/OCL.hpp>

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
    
    class Kuka361nAxesTorqueController : public RTT::TaskContext
    {
    public:
        /** 
         * The contructor of the class.
         * 
         * @param name Name of the TaskContext
         * @param propertyfilename name of the propertyfile to
         * configure the component with, default: cpf/Kuka361nAxesTorqueController.cpf
         * 
         */
        Kuka361nAxesTorqueController(std::string name,std::string propertyfilename="cpf/Kuka361nAxesTorqueController.cpf");
        virtual ~Kuka361nAxesTorqueController();
    
    protected:  
        /** 
         * Method to start an axis .
         *
         * Sets the axis in the DRIVEN state. Only possible if the axis
         * is int the STOPPED state. If succesfull the drive value of
         * the axis is setted to zero and will be updated periodically
         * from the ReadDataPort _driveValue
         * 
         * @param axis nr of the axis to start
         * 
         * @return Can only succeed if the axis was in the DRIVEN state
         */
        Method<bool(int)> _startAxis; 
        
        /**
         * Method to start all axes .
         *
         * Identical to calling startAxis(int axis) on all axes.
         *  
         * @return true if all Axes could be started.
         */
        Method<bool(void)> _startAllAxes; 
        
        /**
         * Method to stop an axis .
         *
         * Sets the drive value to zero and changes to the STOP
         * state. Only possible if axis is in the DRIVEN state. In the
         * stop state, the axis does not listen and write to its 
         * ReadDataPort _driveValue.
         *
         * @param axis nr of the axis to stop
         *
         * @return false if in wrong state or already stopped.
         */
        Method<bool(int)> _stopAxis; 
      
        /** Method to stop all axes.
         *
         * Identical to calling stopAxis(int axis) on all axes.
         */
        Method<bool(void)> _stopAllAxes; 
        
        /** 
         * Method to unlock an axis .
         *
         * Activates the brake of the axis.  Only possible in the STOPPED state.
         * 
         * @param axis nr of the axis to lock
         * 
         * @return false if in wrong state or already locked.
         */
        Method<bool(int)> _unlockAxis; 
        
        /** 
         * Method to unlock all axes 
         *
         * identical to calling lockAxis(int axis) on all axes
         */
        Method<bool(void)> _unlockAllAxes;
        
        /** 
         * Method to lock an axis.
         *
         * Releases the brake of the axis.  Only possible in the LOCKED
         * state.
         * 
         * @param axis nr of axis to unlock
         * 
         * @return false if in wrong state or already locked.
         */
        Method<bool(int)> _lockAxis; 

        /** 
         * Method to lock all axes .
         *
         * identical to unlockAxis(int axis) on all axes;
       */
        Method<bool(void)> _lockAllAxes; 

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
        Command<bool(void)> _prepareForUse; 
        
        /** 
         * Command to Shutdown the hardware controller of the robot.
         * 
         */
        Command<bool(void)> _prepareForShutdown;
        
        /**
         * Method to add a drive offset to an axis.
         *
         * Adds an offset to the _driveValue of axis and updates the
         * _driveOffset value.
         * 
         * @param axis nr of Axis
         * @param offset offset value in fysical units
         * 
         */
        Method<bool(int,double)> _addDriveOffset;

        /**
         * vector of ReadDataPorts which contain the output velocities
         * of the axes.  
         * 
         */
        std::vector<RTT::ReadDataPort<double>*>   _driveValue;

        /**
         * vector of WriteDataPorts which contain the values of the
         * position sensors. It is used by other components who need this
         * value for control ;)
         * 
         */
        std::vector<RTT::DataPort<double>*>       _positionValue;

		/**
         * vector of WriteDataPorts which contain the values of the
         * tachometer. 
         * 
         */
        std::vector<RTT::DataPort<double>*>       _velocityValue;

		/**
         * vector of WriteDataPorts which contain the values of the
         * current sensors transformed to torque. 
         * 
         */
        std::vector<RTT::DataPort<double>*>       _currentValue;

        /**
         * The absolute value of the velocity will be limited to this property.  
         * Used to fire an event if necessary and to saturate the velocities.
         * It is a good idea to set this property to a low value when using experimental code.
         */
        RTT::Property<std::vector <double> >     _velocityLimits;

	/**
         * The absolute value of the current will be limited to this property.  
         * Used to fire an event if necessary and to saturate the currents.
         * It is a good idea to set this property to a low value when using experimental code.
         */
        RTT::Property<std::vector <double> >     _currentLimits; 

	/**
         * Selection of control mode (velocity or torque) for each axis
         */
        RTT::Property<std::vector <double> >     _mode; 

        /**
         * Lower limit for the positions.  Used to fire an event if necessary.
         */
        RTT::Property<std::vector <double> >     _lowerPositionLimits;
        
        /**
         * upper limit for the positions.  Used to fire an event if necessary.
         */
        RTT::Property<std::vector <double> >     _upperPositionLimits;
        
        /**
         *  Start position in rad for simulation.  If the encoders are relative ( like for this component )
         *  also the starting value for the relative encoders.
         */
        RTT::Property<std::vector <double> >     _initialPosition;
        
        /**
         * Offset to the drive value 
         * volt = (setpoint + offset)/scale
         */
        RTT::Property<std::vector <double> >     _velDriveOffset;
      
        /**
         * True if simulationAxes should be used in stead of hardware axes
         */
      
        RTT::Property<bool >     _simulation;
        
        /**
         * Constant: number of axes
         */
        RTT::Constant<unsigned int> _num_axes;
        
        /**
         *  parameters to this event are the axis and the velocity that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue with the previous value.
         */
        RTT::Event< void(std::string) > _velocityOutOfRange;
        
        /**
         *  parameters to this event are the axis and the current that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue with the previous value.
         */
        RTT::Event< void(std::string) > _currentOutOfRange; 
        
        /**
         *  parameters to this event are the axis and the position that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue.  The hardware limit switches can be reached when this
         *  event is not handled.
         */ 
        RTT::Event< void(std::string) > _positionOutOfRange;
        
    private:    

        virtual bool startAxis(int axis);
        virtual bool startAxisCompleted(int axis) const;
    
        virtual bool startAllAxes();
        virtual bool startAllAxesCompleted() const;
    
        virtual bool stopAxis(int axis);
        virtual bool stopAxisCompleted(int axis) const;
    
        virtual bool stopAllAxes();
        virtual bool stopAllAxesCompleted() const;
    
        virtual bool lockAxis(int axis);
        virtual bool lockAxisCompleted(int axis) const;
    
        virtual bool lockAllAxes();
        virtual bool lockAllAxesCompleted() const;
    
        virtual bool unlockAxis(int axis);
        virtual bool unlockAxisCompleted(int axis) const;
    
        virtual bool unlockAllAxes();
        virtual bool unlockAllAxesCompleted() const;
    
        virtual bool addDriveOffset(int axis,double offset);
        virtual bool addDriveOffsetCompleted(int axis) const;
    
        /*
         *Kuka361 does not need a command to initialize the position since
         *it has absolute encoders
         */
        //virtual bool initPosition(int axis);
        //virtual bool initPositionCompleted(int axis) const;
        
        virtual bool prepareForUse();
        virtual bool prepareForUseCompleted() const;
        virtual bool prepareForShutdown();
        virtual bool prepareForShutdownCompleted() const;
    

        /**
         * A local copy of the name of the propertyfile so we can store
         * changed properties.
         */
        const std::string _propertyfile;
    
        /**
         * Activation state of robot
         */
        bool _activated;
    
        /**
         * conversion factor between position value and the encoder input.
         * position = (encodervalue)/scale
         */
        
        std::vector<double>     _positionConvertFactor;
    
    
        /**
         * conversion factor between drive value and the analog output.
         * volt = (setpoint + offset)/scale
         */
        
        std::vector<double>     _driveConvertFactor;

        /**
         * conversion factor between velocity value and the analog input.
         * velocity = (volt - offset)/scale
         */
        
        std::vector<double>     _tachoConvertScale;
        std::vector<double>     _tachoConvertOffset;

         /**
         * Parameters of current regulator
         * I = (a*UN + b)/R
         */
        
        std::vector<double>     _curReg_a;
        std::vector<double>     _curReg_b;
        std::vector<double>     _shunt_R;
        std::vector<double>     _shunt_c;

         /**
         * Motor current constant Km
         * torque = current * Km
         */
        
        std::vector<double>     _Km;
	
         /**
         * Define which axes are torque controlled
         * torque control = 1
         * velocity control = 0
         */
        
        std::vector<double>     _torqueControlled;


    public:
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
        
    private:
        // 
        // Members implementing the interface to the hardware
        //
#if (defined (OROPKG_OS_LXRT))
        std::vector<RTT::Axis*>  _axes_hardware;
        
        RTT::ComediDevice*                    _comediDev;
        RTT::ComediSubDeviceAOut*             _comediSubdevAOut;
        RTT::EncoderSSI_apci1710_board*       _apci1710;
        RTT::RelayCardapci2200*               _apci2200;
        RTT::SwitchDigitalInapci1032*         _apci1032;
        RTT::ComediDevice*                    _comediDev_NI6024; 
        RTT::ComediSubDeviceAIn*              _comediSubdevAIn_NI6024; 
        RTT::ComediSubDeviceDIn*              _comediSubdevDIn_NI6024;
        RTT::ComediDevice*                    _comediDev_NI6527;
        RTT::ComediSubDeviceDOut*             _comediSubdevDOut_NI6527;
        
        std::vector<RTT::EncoderInterface*>           _encoderInterface;
        std::vector<RTT::AbsoluteEncoderSensor*>      _encoder;
        std::vector<RTT::AnalogOutput<unsigned int>*> _ref;
        std::vector<RTT::DigitalOutput*>              _enable;
        std::vector<RTT::AnalogDrive*>                _drive;
        std::vector<RTT::DigitalOutput*>              _brake;
        std::vector<RTT::AnalogInput<unsigned int>*>  _tachoInput; 
        std::vector<RTT::AnalogSensor*>               _tachometer; 
        std::vector<RTT::AnalogInput<unsigned int>*>  _currentInput; 
        std::vector<RTT::AnalogSensor*>               _currentSensor; 
        std::vector<RTT::DigitalOutput*>              _modeSwitch; 
        std::vector<RTT::DigitalInput*>               _modeCheck; 

// 	//Temp
// 	RTT::AnalogInput<unsigned int>*  _motorCurrentInput; 
// 	RTT::AnalogSensor*               _motorCurrentSensor; 
// 	RTT::DataPort<double>*       			_motorCurrentValue;
    
#endif
        std::vector<RTT::AxisInterface*>      _axes;
        std::vector<RTT::AxisInterface*>      _axes_simulation;
	OCL::Kuka361TorqueSimulator*		_torqueSimulator;
        std::vector<double> 			_tau_sim;
	std::vector<double> 			_pos_sim;
	std::vector<TimeService::ticks>		_previous_time;
	TimeService::Seconds 			_delta_time;

    };//class Kuka361nAxesTorqueController
}//namespace Orocos
#endif // Kuka361nAxesTorqueController
