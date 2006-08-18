#ifndef KUKA160_NAXES_VELOCITY_CONTROLLER_HPP
#define KUKA160_NAXES_VELOCITY_CONTROLLER_HPP

#include <pkgconf/system.h> 

#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
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
  public:
    Kuka160nAxesVelocityController(std::string name,std::string propertyfilename="cpf/Kuka160nAxesVelocityController.cpf");
    virtual ~Kuka160nAxesVelocityController();
    
  protected:  
    //
    // Members implementing the component interface
    //
    
    //
    // COMMANDS :
    //
    
    /**
     * \brief Sets the velocity to zero and changes to the ACTIVE state.
     * In the active state, the axis listens and writes to its data-ports.
     */
    virtual bool startAxis(int axis);
    virtual bool startAxisCompleted(int axis) const;
    
    /**
     * identical to calling startAxis on all axes.
     */
    virtual bool startAllAxes();
    virtual bool startAllAxesCompleted() const;
    
    /**
     * \brief Sets the velocity to zero and changes to the STOP state.
     * In the stop state, the axis does not listen and write to its data-port.
     */
    virtual bool stopAxis(int axis);
    virtual bool stopAxisCompleted(int axis) const;
    
    /**
     * \brief identical to calling stopAxis on all axes.
     */
    virtual bool stopAllAxes();
    virtual bool stopAllAxesCompleted() const;
    
    /**
     * Activates the brake of the axis.  Only possible in the STOP state.
     * \return false if in wrong state or already locked.
     */
    virtual bool lockAxis(int axis);
    virtual bool lockAxisCompleted(int axis) const;
    
    /**
     * identical to calling lockAxis on all axes
     */
    virtual bool lockAllAxes();
    virtual bool lockAllAxesCompleted() const;
    
    /**
     * Releases the brake of the axis.  Only possible in the STOP state.
     */
    virtual bool unlockAxis(int axis);
    virtual bool unlockAxisCompleted(int axis) const;
    
    /**
     * identical to unlockAxis(int axis) on all axes;
     */
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
     * The Dataports: 
     */
    
    std::vector<RTT::ReadDataPort<double>*>   _driveValue;
    std::vector<RTT::WriteDataPort<bool>*>    _references;
    std::vector<RTT::WriteDataPort<double>*>  _positionValue;

    /**
     * A local copy of the name of the propertyfile so we can store
     * changed properties.
     */
    const std::string _propertyfile;
    
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
