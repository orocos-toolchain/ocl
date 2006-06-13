#ifndef KUKA160_NAXES_VELOCITY_CONTROLLER_HPP
#define KUKA160_NAXES_VELOCITY_CONTROLLER_HPP


#include <vector>
#include <corelib/RTT.hpp>

#include <execution/GenericTaskContext.hpp>
//#include <corelib/Event.hpp>
#include <execution/DataPort.hpp>


#include <pkgconf/system.h> 

#if (defined (OROPKG_OS_LXRT) && defined (OROPKG_DEVICE_DRIVERS_COMEDI))
//#include <comedi/ComediDevice.hpp>
//#include <comedi/ComediSubDeviceAOut.hpp>
//#include <comedi/ComediSubDeviceDIn.hpp>
//#include <comedi/ComediSubDeviceDOut.hpp>
//#include <comedi/ComediEncoder.hpp>
//#include <device_drivers/IncrementalEncoderSensor.hpp>
//#include <device_drivers/AnalogOutput.hpp>
//#include <device_drivers/DigitalOutput.hpp>
//#include <device_drivers/DigitalInput.hpp>
//#include <device_drivers/AnalogDrive.hpp>
//#include <device_drivers/Axis.hpp>
#endif
#include <device_drivers/SimulationAxis.hpp>
//#include <device_interface/AxisInterface.hpp>

namespace Orocos
{
  class Kuka160nAxesVelocityController : public RTT::GenericTaskContext
  {
  public:
    Kuka160nAxesVelocityController(const std::string propertyfilename="cpf/Kuka160.cpf");
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
    
    //
    std::vector<RTT::WriteDataPort<bool>*>    _references;
    std::vector<RTT::WriteDataPort<double>*>  _positionValue;
    //
    //std::vector<ORO_Execution::WriteDataPort<bool>*>    _homed;
    
  private:
    
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
     *  parameters to this event are the axis and the velocity that is out of range.
     *  Each axis that is out of range throws a seperate event.
     *  The component will continue with the previous value.
     */
    RTT::Event< void(int,double) > _driveOutOfRange;
    RTT::EventC _driveOutOfRange_event;
    int _driveOutOfRange_axis;
    double _driveOutOfRange_value;
        
    /**
     *  parameters to this event are the axis and the position that is out of range.
     *  Each axis that is out of range throws a seperate event.
     *  The component will continue.  The hardware limit switches can be reached when this
     *  event is not handled.
     */ 
    RTT::Event< void(int,double) > _positionOutOfRange;
    RTT::EventC _positionOutOfRange_event;
    int _positionOutOfRange_axis;
    double _positionOutOfRange_value;
    
  private:
    /**
     * Extra private variables
     */
    
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
    std::vector<ORO_DeviceDriver::Axis*>               _axes;
    RTT::ComediDevice*                    _comediDevAOut;
    RTT::ComediDevice*                    _comediDevEncoder;
    RTT::ComediDevice*                    _comediDevDInOut;
    RTT::ComediSubDeviceAOut*             _comediSubdevAOut;
    RTT::ComediSubDeviceDIn*              _comediSubdevDIn;
    RTT::ComediSubDeviceDOut*             _comediSubdevDOut;
    RTT::EncoderInterface*             _encoderInterface[KUKA160_NUM_AXIS];
  
    RTT::AnalogOutput<unsigned int>*      _vref[KUKA160_NUM_AXIS];
    RTT::IncrementalEncoderSensor*        _encoder[KUKA160_NUM_AXIS];
    RTT::DigitalOutput*                   _enable[KUKA160_NUM_AXIS];
    RTT::AnalogDrive*                     _drive[KUKA160_NUM_AXIS];
    RTT::DigitalOutput*                   _brake[KUKA160_NUM_AXIS];
    RTT::DigitalInput*                    _reference[KUKA160_NUM_AXIS];  
#else
    std::vector<RTT::SimulationAxis*>      _axes;
#endif
    std::vector<RTT::AxisInterface*>    _axesInterface;
    
  };//class Kuka160nAxesVelocityController
}//namespace Orocos
#endif // KUKA160NAXESVELOCITYCONTROLLER
