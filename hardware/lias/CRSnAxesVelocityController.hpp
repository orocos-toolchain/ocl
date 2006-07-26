#ifndef LIAS_NAXES_VELOCITY_CONTROLLER_HPP
#define LIAS_NAXES_VELOCITY_CONTROLLER_HPP
#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>


#include <pkgconf/system.h> 

#include "IP_Encoder_6_EncInterface.hpp"
#include <rtt/dev/SimulationAxis.hpp> 


#if defined (OROPKG_OS_LXRT)

    #include "CombinedDigitalOutInterface.hpp"

    #include <rtt/dev/IncrementalEncoderSensor.hpp>
    #include <rtt/dev/AnalogOutput.hpp>
    #include <rtt/dev/DigitalOutput.hpp>
    #include <rtt/dev/DigitalInput.hpp>
    #include <rtt/dev/AnalogDrive.hpp>
    #include <rtt/dev/Axis.hpp>
    #include <rtt/dev/AxisInterface.hpp>
#endif

#include "LiASConstants.hpp"

namespace Orocos {

class CRSnAxesVelocityController : public RTT::GenericTaskContext
{
public:
   CRSnAxesVelocityController(const std::string& name,const std::string& propertyfilename="cpf/crs.cpf");
  virtual ~CRSnAxesVelocityController();

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

  // METHOD returning the status of each axis
  virtual bool isDriven(int axis);

private:
  std::vector<RTT::ReadDataPort<double>*>   driveValue;

  std::vector<RTT::WriteDataPort<bool>*>    reference;
  std::vector<RTT::WriteDataPort<double>*>  positionValue;
  std::vector<RTT::WriteDataPort<double>*>  output;

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
   * Sign for each axis.  Double, should have only -1.0 and 1.0 values.
   */
  RTT::Property<std::vector<double> >      signAxes;

  /**
   * Offset for each axis to compensate friction.  Should only partially compensate friction.
   */
  RTT::Property<std::vector<double> >      offset;


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

   /**
    * Constant : number of axes
    */
   RTT::Constant<unsigned int> _num_axes;

private:
  // to keep track of which axes are homed :
  std::vector<bool>  _homed;	
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

  /**
   * Derivative action for each axis.
   */
  RTT::Property<std::vector<double> >     servoDerivTime; 

  //
  // Continuous state for the servo-loop
  //
  std::vector<double>  servoIntVel;     // integrated velocity
  std::vector<double>  servoIntError;   // integrated error
  bool                               servoInitialized;
  RTT::TimeService::ticks    previousTime;
  std::vector<double>        previousPos;  // used by friction comp.

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
     std::vector<RTT::SimulationAxis*>      _axes;
     std::vector<RTT::AxisInterface*>    _axesInterface;
  #else
    std::vector<RTT::Axis*>                _axes;

    std::vector<RTT::AxisInterface*>    _axesInterface;
  
    RTT::DigitalOutInterface*               _IP_Digital_24_DOut;
    IP_Encoder_6_Task*                                      _IP_Encoder_6_task;
    RTT::AnalogOutInterface<unsigned int>*  _IP_FastDac_AOut;
    RTT::DigitalInInterface*                _IP_OptoInput_DIn;
  
    RTT::DigitalOutput*                    _enable;
    RTT::DigitalOutput*                    _combined_enable[LiAS_NUM_AXIS];
    RTT::CombinedDigitalOutInterface*   _combined_enable_DOutInterface;
    RTT::DigitalOutput*                    _brake;
    RTT::DigitalOutput*                    _combined_brake[2];
    RTT::CombinedDigitalOutInterface*   _combined_brake_DOutInterface;

    RTT::EncoderInterface*              _encoderInterface[LiAS_NUM_AXIS];
    RTT::IncrementalEncoderSensor*         _encoder[LiAS_NUM_AXIS];
    RTT::AnalogOutput<unsigned int>*       _vref[LiAS_NUM_AXIS];
    RTT::AnalogDrive*                      _drive[LiAS_NUM_AXIS];
    RTT::DigitalInput*                     _reference[LiAS_NUM_AXIS];  
  #endif
  
  bool _activate_axis2, _activate_axis3, _deactivate_axis2, _deactivate_axis3;

};

} // namespace Orocos

#endif // LIAS_HARDWARE_HPP
