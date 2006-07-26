#ifndef KUKA361_NAXES_VELOCITY_CONTROLLER_HPP
#define KUKA361_NAXES_VELOCITY_CONTROLLER_HPP


#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>

#include <pkgconf/system.h> 

#if (defined (OROPKG_OS_LXRT) && defined (OROPKG_DEVICE_DRIVERS_COMEDI)) && defined (OROPKG_DEVICE_DRIVERS_APCI)
#include <rtt/dev/SwitchDigitalInapci1032.hpp>
#include <rtt/dev/RelayCardapci2200.hpp>
#include <rtt/dev/EncoderSSIapci1710.hpp>
#include <rtt/dev/ComediDevice.hpp>
#include <rtt/dev/ComediSubDeviceAOut.hpp>

#include <rtt/dev/AbsoluteEncoderSensor.hpp>
#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>
#include <rtt/dev/AnalogDrive.hpp>
#include <rtt/dev/Axis.hpp>
#endif
#include <rtt/dev/SimulationAxis.hpp>
#include <rtt/dev/AxisInterface.hpp>

namespace Orocos
{
  class Kuka361nAxesVelocityController : public RTT::GenericTaskContext
  {
  public:
    Kuka361nAxesVelocityController(std::string name,std::string propertyfilename="cpf/Kuka361nAxesVelocityController.cpf");
    virtual ~Kuka361nAxesVelocityController();
    
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
    
  private:
    /**
     * The Dataports: 
     */
    
    std::vector<RTT::ReadDataPort<double>*>   _driveValue;
    
    /*
     *Kuka361 does has no reference signals
    */
    //std::vector<RTT::WriteDataPort<bool>*>    _references;
    std::vector<RTT::WriteDataPort<double>*>  _positionValue;
    //
    //std::vector<RTT::WriteDataPort<bool>*>    _homed;
    
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
     * Constant: number of axes
     */
    RTT::Constant<unsigned int> _num_axes;
    
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
#if  (defined (OROPKG_OS_LXRT) && defined (OROPKG_DEVICE_DRIVERS_COMEDI))&& defined (OROPKG_DEVICE_DRIVERS_APCI)
    std::vector<RTT::Axis*>  _axes_hardware;

    RTT::ComediDevice*                    _comediDev;
    RTT::ComediSubDeviceAOut*             _comediSubdevAOut;
    RTT::EncoderSSI_apci1710_board*       _apci1710;
    RTT::RelayCardapci2200*               _apci2200;
    RTT::SwitchDigitalInapci1032*         _apci1032;

    RTT::EncoderInterface*                _encoderInterface[6];
    RTT::AbsoluteEncoderSensor*           _encoder[6];
    RTT::AnalogOutput<unsigned int>*      _vref[6];
    RTT::DigitalOutput*                   _enable[6];
    RTT::AnalogDrive*                     _drive[6];
    RTT::DigitalOutput*                   _brake[6];
    
#endif
    std::vector<RTT::SimulationAxis*>     _axes_simulation;
    std::vector<RTT::AxisInterface*>      _axes;
    
  };//class Kuka361nAxesVelocityController
}//namespace Orocos
#endif // KUKA361NAXESVELOCITYCONTROLLER
