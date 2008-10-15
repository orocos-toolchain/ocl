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

#include <ocl/dev/SimulationAxis.hpp>
#include <ocl/dev/TorqueSimulationAxis.hpp>
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
        Kuka361nAxesTorqueController(std::string name);
        virtual ~Kuka361nAxesTorqueController();
    
    protected:  
        /**
         * vector of ReadDataPorts which contain the output velocities
         * of the axes.  
         * 
         */
        RTT::DataPort<std::vector<double> >   driveValues;
        std::vector<double> driveValues_local;
        /**
         * vector of WriteDataPorts which contain the values of the
         * position sensors. It is used by other components who need this
         * value for control ;)
         * 
         */
        RTT::DataPort<std::vector<double> >       positionValues;
        std::vector<double> positionValues_local;
        
        /**
         * vector of WriteDataPorts which contain the values of the
         * tachometer. 
         * 
         */
        RTT::DataPort<std::vector<double> >       velocityValues;
        std::vector<double> velocityValues_local;
		/**
         * vector of WriteDataPorts which contain the values of the
         * current sensors transformed to torque. 
         * 
         */
        RTT::DataPort<std::vector<double> >       torqueValues;
        std::vector<double> torqueValues_local;
        /**
         * Port containing time since previous update (samole time)
         * 
         */	
        RTT::WriteDataPort< double >		delta_time_Port;


        /**
         * The absolute value of the velocity will be limited to this property.  
         * Used to fire an event if necessary and to saturate the velocities.
         * It is a good idea to set this property to a low value when using experimental code.
         */
        RTT::Property<std::vector <double> >     velocityLimits;

        /**
         * The absolute value of the current will be limited to this property.  
         * Used to fire an event if necessary and to saturate the currents.
         * It is a good idea to set this property to a low value when using experimental code.
         */
        RTT::Property<std::vector <double> >     currentLimits; 


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
         * Offset to the drive value 
         * volt = (setpoint + offset)/scale
         */
        RTT::Property<std::vector <double> >     velDriveOffset;
      
        /**
         * True if simulationAxes should be used in stead of hardware axes
         */
      
        RTT::Property<bool>     simulation;
        
        /**
         * Constant: number of axes
         */
        RTT::Constant<unsigned int> num_axes;
        
        /**
         *  parameters to this event are the axis and the velocity that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue with the previous value.
         */
        RTT::Event< void(std::string) > velocityOutOfRange;
        
        /**
         *  parameters to this event are the axis and the current that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue with the previous value.
         */
        RTT::Event< void(std::string) > currentOutOfRange; 
        
        /**
         *  parameters to this event are the axis and the position that is out of range.
         *  Each axis that is out of range throws a seperate event.
         *  The component will continue.  The hardware limit switches can be reached when this
         *  event is not handled.
         */ 
        RTT::Event< void(std::string) > positionOutOfRange;
        
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
    

        /**
         * If true robot will be velocity controlled
         */
        RTT::Property<bool> velresolved;

        /**
         * Selection of control mode (velocity = 0 or torque = 1) for each axis
         */
        bool TorqueMode;

    
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

        /**
         * conversion factor between velocity value and the analog input.
         * velocity = (volt - offset)/scale
         */
        
        std::vector<double>     tachoConvertScale;
        std::vector<double>     tachoConvertOffset;

         /**
         * Parameters of current regulator
         * I = (a*UN + b)/R
         */
        
        std::vector<double>     curReg_a;
        std::vector<double>     curReg_b;
        std::vector<double>     shunt_R;
        std::vector<double>     shunt_c;

         /**
         * Motor current constant Km
         * torque = current * Km
         */
        
        std::vector<double>     Km;

        /**
         * List of Events that should result in an emergencystop
         */
        RTT::Property<std::vector<std::string> > EmergencyEvents_prop;
        

    public:
        virtual bool configureHook();
        
        /**
         *  This function contains the application's startup code.
         *  Return false to abort startup.
         **/
        virtual bool startHook(); 
        
        /**
         * This function is periodically called.
         */
        virtual void updateHook();
        
        /**
         * This function is called when the task is stopped.
         */
        virtual void stopHook();
        
        virtual void cleanupHook();
        
    private:

        void EmergencyStop(std::string message)
            {
                log(Error) << "---------------------------------------------" << endlog();
                log(Error) << "--------- EMERGENCY STOP --------------------" << endlog();
                log(Error) << "---------------------------------------------" << endlog();
                log(Error) << message << endlog();
                this->fatal();
            };
        std::vector<RTT::Handle> EmergencyEventHandlers;
        
        // 
        // Members implementing the interface to the hardware
        //
#if (defined (OROPKG_OS_LXRT))
        std::vector<RTT::Axis*>          axes_hardware;
        
        ComediDevice*                    comediDev;
        ComediSubDeviceAOut*             comediSubdevAOut;
        EncoderSSI_apci1710_board*       apci1710;
        RelayCardapci2200*               apci2200;
        SwitchDigitalInapci1032*         apci1032;
        ComediDevice*                    comediDev_NI6024; 
        ComediSubDeviceAIn*              comediSubdevAIn_NI6024; 
        ComediSubDeviceDIn*              comediSubdevDIn_NI6024;
        ComediSubDeviceDOut*             comediSubdevDOut_NI6713;
        
        std::vector<RTT::EncoderInterface*>           encoderInterface;
        std::vector<RTT::AbsoluteEncoderSensor*>      encoder;
        std::vector<RTT::AnalogOutput*>               ref;
        std::vector<RTT::DigitalOutput*>              enable;
        std::vector<RTT::AnalogDrive*>                drive;
        std::vector<RTT::DigitalOutput*>              brake;
        std::vector<RTT::AnalogInput*>                tachoInput; 
        std::vector<RTT::AnalogSensor*>               tachometer; 
        std::vector<RTT::AnalogInput*>                currentInput; 
        std::vector<RTT::AnalogSensor*>               currentSensor; 
        RTT::DigitalOutput*                           torqueModeSwitch;
        std::vector<RTT::DigitalInput*>               torqueModeCheck; 
        
        
#endif
        std::vector<RTT::AxisInterface*>      axes;
        std::vector<RTT::AxisInterface*>      axes_simulation;
        OCL::Kuka361TorqueSimulator*		  torqueSimulator;
        std::vector<double> 			      tau_sim;
        std::vector<double> 			      pos_sim;
        std::vector<TimeService::ticks>		  previous_time;
        TimeService::Seconds 			      delta_time;
        
    };//class Kuka361nAxesTorqueController
}//namespace Orocos
#endif // Kuka361nAxesTorqueController
