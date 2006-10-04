#ifndef KUKA160_NAXES_VELOCITY_CONTROLLER_HPP
#define KUKA160_NAXES_VELOCITY_CONTROLLER_HPP

#include <pkgconf/system.h> 

#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Command.hpp>
#include <rtt/Properties.hpp>

#if (defined (OROPKG_OS_LXRT) && defined (OROPKG_DEVICE_DRIVERS_COMEDI))
#include <rtt/dev/ComediDevice.hpp>
#include <rtt/dev/ComediSubDeviceAOut.hpp>
#include <rtt/dev/ComediSubDeviceDIn.hpp>
#include <rtt/dev/ComediSubDeviceDOut.hpp>
#include <rtt/dev/ComediEncoder.hpp>
#include <rtt/dev/IncrementalEncoderSensor.hpp>
#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>
#include <rtt/dev/DigitalInput.hpp>
#include <rtt/dev/AnalogDrive.hpp>
#include <rtt/dev/Axis.hpp>
#endif
#include <rtt/dev/SimulationAxis.hpp>
#include <rtt/dev/AxisInterface.hpp>

namespace Orocos
{
    /**
     * This class implements a GenericTaskContext to use with the
     * Kuka160 robot in the RoboticLab, PMA, dept. Mechanical
     * Engineering, KULEUVEN. Since the hardware part is very specific
     * for our setup, other people can only use the simulation
     * version. But it can be a good starting point to create your own
     * Robot Software Interface.
     * 
     */

  class Kuka160nAxesVelocityController : public RTT::GenericTaskContext
  {
  public:/** 
          * The contructor of the class.
          * 
          * @param name Name of the TaskContext
          * @param propertyfilename name of the propertyfile to
          * configure the component with, default: cpf/Kuka160nAxesVelocityController.cpf
          * 
          */
      
      Kuka160nAxesVelocityController(std::string name,std::string propertyfilename="cpf/Kuka160nAxesVelocityController.cpf");
      virtual ~Kuka160nAxesVelocityController();
    

  protected:
      /** 
       * Command to start an axis .
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
      Command<bool(int)> _startAxis; 
      
      /**
       * Command to start all axes .
       *
       * Identical to calling startAxis(int axis) on all axes.
       *  
       * @return true if all Axes could be started.
       */
      Command<bool(void)> _startAllAxes; 
      
      /**
       * Command to stop an axis .
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
      Command<bool(int)> _stopAxis; 
      
      /** Command to stop all axes.
       *
       * Identical to calling stopAxis(int axis) on all axes.
       */
      Command<bool(void)> _stopAllAxes; 
      
      /** 
       * Command to unlock an axis .
       *
       * Activates the brake of the axis.  Only possible in the STOPPED state.
       * 
       * @param axis nr of the axis to lock
       * 
       * @return false if in wrong state or already locked.
       */
      Command<bool(int)> _unlockAxis; 

      /** 
       * Command to unlock all axes 
       *
       * identical to calling lockAxis(int axis) on all axes
       */
      Command<bool(void)> _unlockAllAxes;
      
      /** 
       * Command to lock an axis.
       *
       * Releases the brake of the axis.  Only possible in the LOCKED
       * state.
       * 
       * @param axis nr of axis to unlock
       * 
       * @return false if in wrong state or already locked.
       */
      Command<bool(int)> _lockAxis; 

      /** 
       * Command to lock all axes .
       *
       * identical to unlockAxis(int axis) on all axes;
       */
      Command<bool(void)> _lockAllAxes; 

      /**
       * Command to prepare robot for use.
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
       * Command to add a drive offset to an axis.
       *
       * Adds an offset to the _driveValue of axis and updates the
       * _driveOffset value.
       * 
       * @param axis nr of Axis
       * @param offset offset value in fysical units
       * 
       */
      Command<bool(int,double)> _addDriveOffset;

      /**
       * Command to set the position of an axis to its initial
       * position.
       *
       * Sets the _positionValue to the _initialPosition from the
       * property-file. This is needed because the Kuka160 needs to be homed.
       * 
       * @param axis nr of axis
       */
      Command<bool(int)>  _initPosition; 

      /**
       * vector of ReadDataPorts which contain the output velocities
       * of the axes.  
       * 
       */
      std::vector<RTT::ReadDataPort<double>*>   _driveValue;
      /**
       * vector of WriteDataPorts which contain the values of the
       * reference switch. It can be used for the homing of the robot.
       * 
       */
      std::vector<RTT::WriteDataPort<bool>*>    _references;
      /**
       * vector of WriteDataPorts which contain the values of the
       * position sensors. It is used by other components who need this
       * value for control ;)
       * 
       */
      std::vector<RTT::WriteDataPort<double>*>  _positionValue;
      
    
      /**
       * The absolute value of the velocity will be limited to this property.  
       * Used to fire an event if necessary and to saturate the velocities.
       * It is a good idea to set this property to a low value when using experimental code.
       */
      RTT::Property<std::vector <double> >     _driveLimits;
      
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
      
      RTT::Property<std::vector <double> >     _driveOffset;
      
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
      RTT::Event< void(std::string) > _driveOutOfRange;
      
      /**
       *  parameters to this event are the axis and the position that is out of range.
       *  Each axis that is out of range throws a seperate event.
       *  The component will continue.  The hardware limit switches can be reached when this
       *  event is not handled.
       */ 
      RTT::Event< void(std::string) > _positionOutOfRange;
      
  private:  
      //
      // Members implementing the component interface
      //
    
      //
      // COMMANDS :
      //
    
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
      
      virtual bool initPosition(int axis);
      virtual bool initPositionCompleted(int axis) const;
    
      virtual bool prepareForUse();
      virtual bool prepareForUseCompleted() const;

      virtual bool prepareForShutdown();
      virtual bool prepareForShutdownCompleted() const;
    
  private:

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
      
      // 
      // Members implementing the interface to the hardware
      //
#if  (defined (OROPKG_OS_LXRT) && defined (OROPKG_DEVICE_DRIVERS_COMEDI))
      RTT::ComediDevice*                    _comediDevAOut;
      RTT::ComediDevice*                    _comediDevEncoder;
      RTT::ComediDevice*                    _comediDevDInOut;
      RTT::ComediSubDeviceAOut*             _comediSubdevAOut;
      RTT::ComediSubDeviceDIn*              _comediSubdevDIn;
      RTT::ComediSubDeviceDOut*             _comediSubdevDOut;
      std::vector<RTT::EncoderInterface*>   _encoderInterface;
  
      std::vector<RTT::AnalogOutput<unsigned int>*> _vref;
      std::vector<RTT::IncrementalEncoderSensor*>   _encoder;
      std::vector<RTT::DigitalOutput*>      _enable;
      std::vector<RTT::AnalogDrive*>        _drive;
      std::vector<RTT::DigitalOutput*>      _brake;
      std::vector<RTT::DigitalInput*>       _reference;  
      std::vector<RTT::Axis*>               _axes_hardware;
#endif
      std::vector<RTT::AxisInterface*>      _axes;
      std::vector<RTT::SimulationAxis*>     _axes_simulation;

    
  };//class Kuka160nAxesVelocityController
}//namespace Orocos
#endif // KUKA160NAXESVELOCITYCONTROLLER
