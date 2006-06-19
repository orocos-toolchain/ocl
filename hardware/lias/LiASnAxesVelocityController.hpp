#ifndef LIAS_NAXES_VELOCITY_CONTROLLER_HPP
#define LIAS_NAXES_VELOCITY_CONTROLLER_HPP
#include <vector>
#include <corelib/RTT.hpp>

#include <execution/GenericTaskContext.hpp>
#include <execution/Ports.hpp>
#include <corelib/Event.hpp>
#include <corelib/Properties.hpp>


#include <pkgconf/system.h> 

#include "interfaces/IP_Encoder_6_EncInterface.hpp"
#include <device_drivers/SimulationAxis.hpp> 


#if defined (OROPKG_OS_LXRT)

    #include "interfaces/CombinedDigitalOutInterface.hpp"

    #include <device_drivers/IncrementalEncoderSensor.hpp>
    #include <device_drivers/AnalogOutput.hpp>
    #include <device_drivers/DigitalOutput.hpp>
    #include <device_drivers/DigitalInput.hpp>
    #include <device_drivers/AnalogDrive.hpp>
    #include <device_drivers/Axis.hpp>
    #include <device_interface/AxisInterface.hpp>
#endif

#include "LiASConstants.hpp"

namespace Orocos {

class LiASnAxesVelocityController : public ORO_Execution::GenericTaskContext
{
public:
   LiASnAxesVelocityController(const std::string& name,const std::string& propertyfilename="cpf/lias.cpf");
  virtual ~LiASnAxesVelocityController();

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
  virtual bool addDriveOffsetCompleted(int axis,double offset) const;

  // initPosition : argument q gegeven via initialPosition.
  virtual bool initPosition(int axis);
  virtual bool initPositionCompleted(int axis) const;

  virtual bool prepareForUse();
  virtual bool prepareForUseCompleted() const;
  virtual bool prepareForShutdown();
  virtual bool prepareForShutdownCompleted() const;

private:
  std::vector<ORO_Execution::ReadDataPort<double>*>   driveValue;

  std::vector<ORO_Execution::WriteDataPort<bool>*>    reference;
  std::vector<ORO_Execution::WriteDataPort<double>*>  positionValue;

private:  
  /**
   * A local copy of the name of the propertyfile so we can store changed
   * properties.
   */
 const std::string _propertyfile;

 /**
  * The absolute value of the velocity will be limited to this property.  
  * Used to fire an event if necessary and to saturate the velocities.
  * It is a good idea to set this property to a low value when using experimental code.
  */
  RTT::Property<std::vector <double> >     driveLimits;

  /**
   * Lower limit for the positions.  Used to fire an event if necessary.
   */
  RTT::Property<std::vector <double> >     lowerPositionLimits;

  /**
   * upper limit for the positions.  Used to fire an event if necessary.
   */
  RTT::Property<std::vector <double> >     upperPositionLimits;

  /**
   *  Start position in rad for simulation.  If the encoders are relative ( like for this component )
   *  also the starting value for the relative encoders.
   */
  RTT::Property<std::vector <double> >     initialPosition;


   /**
    *  parameters to this event are the axis and the velocity that is out of range.
    *  Each axis that is out of range throws a seperate event.
    *  The component will continue with a saturated value.
    */
  RTT::Event< void(int,double) > driveOutOfRange;
  RTT::EventC                    driveOutOfRange_eventc;
  int                                    driveOutOfRange_axis;
  double 								 driveOutOfRange_value;

   /**
    *  parameters to this event are the axis and the position that is out of range.
    *  Each axis that is out of range throws a seperate event.
    *  The component will continue.  The hardware limit switches can be reached when this
    *  event is not handled.
    */ 
  RTT::Event< void(int,double) > positionOutOfRange;
  RTT::EventC				     positionOutOfRange_eventc;
  int									 positionOutOfRange_axis;
  double							     positionOutOfRange_value;

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
  // Private properties of this component.
  //
  //
  
  /**
   * conversion factor between drive value and the analog output.
   * volt = (setpoint + offset)/scale
   *
   * RTT::Property<std::vector <double> >     driveConvertFactor;
   */

  /**
   * Offset to the drive value 
   * volt = (setpoint + offset)/scale
   *
   * RTT::Property<std::vector <double> >     driveOffset;
   */



private:
  //
  //   Servo-loop gain :
  //   property to indicate initial value and variable to store actual value
  
  RTT::Property<std::vector <double> >     servoGain;
  std::vector<double>                              _servoGain;
  //
  //   Servo-loop integration time
  //   property to indicate initial value and variable to store actual value
  RTT::Property<std::vector <double> >     servoIntegrationFactor;
  std::vector<double>                              _servoIntegrationFactor;

  //
  //   Feedforward scale factor
  //   (mainly to turn feedforward on and off).
  //   property to indicate initial value and variable to store actual value
  RTT::Property<std::vector <double> >     servoFFScale;
  std::vector<double>                              _servoFFScale;

  //
  // Continuous state for the servo-loop
  //
  std::vector<double>  servoIntVel;     // integrated velocity
  std::vector<double>  servoIntError;   // integrated error
  bool                               servoInitialized;
  RTT::TimeService::ticks    previousTime;

  //
  // Command to read and apply the properties to the controller
  //
  virtual bool changeServo();
  virtual bool changeServoCompleted() const;
 
private:
  // 
  // Members implementing the interface to the hardware
  //
  #if !defined (OROPKG_OS_LXRT)
     std::vector<ORO_DeviceDriver::SimulationAxis*>      _axes;
     std::vector<ORO_DeviceInterface::AxisInterface*>    _axesInterface;
  #else
    std::vector<ORO_DeviceDriver::Axis*>                _axes;

    std::vector<ORO_DeviceInterface::AxisInterface*>    _axesInterface;
  
    ORO_DeviceInterface::DigitalOutInterface*               _IP_Digital_24_DOut;
    IP_Encoder_6_Task*                                      _IP_Encoder_6_task;
    ORO_DeviceInterface::AnalogOutInterface<unsigned int>*  _IP_FastDac_AOut;
    ORO_DeviceInterface::DigitalInInterface*                _IP_OptoInput_DIn;
  
    ORO_DeviceDriver::DigitalOutput*                    _enable;
    ORO_DeviceDriver::DigitalOutput*                    _combined_enable[LiAS_NUM_AXIS];
    ORO_DeviceInterface::CombinedDigitalOutInterface*   _combined_enable_DOutInterface;
    ORO_DeviceDriver::DigitalOutput*                    _brake;
    ORO_DeviceDriver::DigitalOutput*                    _combined_brake[2];
    ORO_DeviceInterface::CombinedDigitalOutInterface*   _combined_brake_DOutInterface;

    ORO_DeviceInterface::EncoderInterface*              _encoderInterface[LiAS_NUM_AXIS];
    ORO_DeviceDriver::IncrementalEncoderSensor*         _encoder[LiAS_NUM_AXIS];
    ORO_DeviceDriver::AnalogOutput<unsigned int>*       _vref[LiAS_NUM_AXIS];
    ORO_DeviceDriver::AnalogDrive*                      _drive[LiAS_NUM_AXIS];
    ORO_DeviceDriver::DigitalInput*                     _reference[LiAS_NUM_AXIS];  
  #endif
  
  bool _activate_axis2, _activate_axis3, _deactivate_axis2, _deactivate_axis3;

};

} // namespace Orocos

#endif // LIAS_HARDWARE_HPP
