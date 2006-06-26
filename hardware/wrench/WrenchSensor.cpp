#include "WrenchSensor.h"
#include <corelib/Attribute.hpp>
#include <execution/TemplateFactories.hpp>
#include <geometry/frames.h>

#define CUTOFF_FREQUENCY_FILTER1 500.0
#define CUTOFF_FREQUENCY_FILTER2 125.0
#define CUTOFF_FREQUENCY_FILTER3  31.25
#define CUTOFF_FREQUENCY_FILTER4   7.813
#define CUTOFF_FREQUENCY_FILTER5   1.953
#define CUTOFF_FREQUENCY_FILTER6   0.4883

// 12000 is more or less 75% of 16384
// and 16384 is half of 32768, which is the full range (e.g. -200 to 200 N).
#define MAX_LOAD 12000

using namespace RTT;
using namespace std;
using namespace ORO_Geometry;


WrenchSensor::WrenchSensor(double samplePeriod,std::string name,unsigned int DSP) 
        : RTT::GenericTaskContext(name),
          outdatPort("WrenchData"),
	  _filterToReadFrom(6),
	  _dsp(DSP)
    {
        this->ports()->addPort( &outdatPort );

        /**
         * Method Factory Interface.
         */
        TemplateMethodFactory<WrenchSensor>* fact =
            newMethodFactory( this );

	_writeBuffer = new Wrench(Wrench::Zero());
        _offset = Wrench::Zero();
	
	fact->add( "minMeasurement",
                   method( &WrenchSensor::minMeasurement, "Gets the minimum measurement value." ) );

	fact->add( "maxMeasurement",
                   method( &WrenchSensor::maxMeasurement, "Gets the maximum measurement value." ) );

	fact->add( "zeroMeasurement",
                   method( &WrenchSensor::zeroMeasurement, "Gets the zero measurement value." ) );
		 
   
        this->methods()->registerObject("this", fact);
		
	TemplateCommandFactory<WrenchSensor>* cfact =
	    newCommandFactory( this );

        cfact->add( "chooseFilter",
	            command( &WrenchSensor::chooseFilter,
		                 &WrenchSensor::chooseFilterDone,
			     "Command to choose a different filter","p","periodValue" ) );	
	
        cfact->add( "setOffset",
	            command( &WrenchSensor::setOffset,
		                 &WrenchSensor::setOffsetDone,
			     "Command to set the zero offset","o","offset vector" ) );	
	
	this->commands()->registerObject("this", cfact);
	
	this->events()->addEvent( "maximumLoadEvent", &maximumLoadEvent);

    #if defined (OROPKG_OS_LXRT)            
    chooseFilter(samplePeriod);
    assert(JR3DSP_check_sensor_and_DSP( _dsp));
	JR3DSP_set_units(1, _dsp);
	
	JR3DSP_get_full_scale(&_full_scale, _dsp);
	
        Logger::log() << Logger::Info << "WrenchSensor -  Full scale: ("
                << (double) _full_scale.Fx << ", " << (double) _full_scale.Fy << ", " << (double) _full_scale.Fz
                << (double) _full_scale.Tx << ", " << (double) _full_scale.Ty << ", " << (double) _full_scale.Tz << ")" << Logger::endl;
    #else
        _full_scale.Fx=100.0;
        _full_scale.Fy=100.0;
        _full_scale.Fz=100.0;
        _full_scale.Tx=100.0;
        _full_scale.Ty=100.0;
        _full_scale.Tz=100.0;
    #endif				


        /*if (!readProperties("tcp.cpf")) {
                     Logger::log() << Logger::Error << "Failed to read the property file." << Logger::endl;
                     assert(0);
         }*/
        
                     
    }
  
    ORO_Geometry::Wrench WrenchSensor::maxMeasurement() const
    {
       return ORO_Geometry::Wrench( ORO_Geometry::Vector((double)_full_scale.Fx     , (double)_full_scale.Fy     , (double)_full_scale.Fz)
                                  , ORO_Geometry::Vector((double)_full_scale.Tx / 10, (double)_full_scale.Fy / 10, (double)_full_scale.Fz / 10) );
    }
    
    
    ORO_Geometry::Wrench WrenchSensor::minMeasurement() const
    {
       return ORO_Geometry::Wrench( -ORO_Geometry::Vector((double)_full_scale.Fx     , -(double)_full_scale.Fy     , -(double)_full_scale.Fz)
                                  , -ORO_Geometry::Vector((double)_full_scale.Tx / 10, -(double)_full_scale.Fy / 10, -(double)_full_scale.Fz / 10) );
					
    }
    
    ORO_Geometry::Wrench WrenchSensor::zeroMeasurement() const
    {
       return ORO_Geometry::Wrench(ORO_Geometry::Vector(0,0,0), ORO_Geometry::Vector(0,0,0));
    }
    
    bool WrenchSensor::chooseFilter(double period)
    {
      #if defined (OROPKG_OS_LXRT) 
      // Calculate the best filter to read from, based on the value 'T'
      if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER1)) _filterToReadFrom = 1;
         else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER2)) _filterToReadFrom = 2;
         else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER3)) _filterToReadFrom = 3;
         else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER4)) _filterToReadFrom = 4;
         else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER5)) _filterToReadFrom = 5;
         else if ( period < 1.0/(2*CUTOFF_FREQUENCY_FILTER6)) _filterToReadFrom = 6;
         else
         {
           Logger::log() << Logger::Warning << "(WrenchSensor)  Sample to low to garantee no aliasing!" << Logger::endl;
         }
      #endif 
       Logger::log() << Logger::Info << "WrenchSensor - ChooseFilter: " << _filterToReadFrom << Logger::endl;
       
    }
    
    bool WrenchSensor::chooseFilterDone() const
    {
    
      return true;
    }


    bool WrenchSensor::setOffset(ORO_Geometry::Wrench off)
    {
	
       _offset=off;
       return true;
    }

    bool WrenchSensor::setOffsetDone() const
   {

      return true;
   } 

    /**
     * This function contains the application's startup code.
     * Return false to abort startup.
     */
    bool WrenchSensor::startup() {

      return true;
         
    }

    /**
     * This function is periodically called.
     */
    void WrenchSensor::update() {

      #if defined (OROPKG_OS_LXRT)
      JR3DSP_get_data(&_write_struct, _filterToReadFrom, _dsp);
       
        // Check for overload
      if (  (_write_struct.Fx > MAX_LOAD) || (_write_struct.Fx < -MAX_LOAD)
         || (_write_struct.Fy > MAX_LOAD) || (_write_struct.Fy < -MAX_LOAD)
         || (_write_struct.Fz > MAX_LOAD) || (_write_struct.Fz < -MAX_LOAD)
         || (_write_struct.Tx > MAX_LOAD) || (_write_struct.Tx < -MAX_LOAD)
         || (_write_struct.Ty > MAX_LOAD) || (_write_struct.Ty < -MAX_LOAD)
         || (_write_struct.Tz > MAX_LOAD) || (_write_struct.Tz < -MAX_LOAD) )
	 {
	   
	   //EventC maxLoadEvent  = this.events()->setupEmit( "maximumLoadEvent" );
	   //maxLoadEvent.emit();				       
       maximumLoadEvent.emit();
	 }
     #else
      _write_struct.Fx = 0.0;
      _write_struct.Fy = 0.0;
      _write_struct.Fz = 0.0;
      _write_struct.Tx = 0.0;
      _write_struct.Ty = 0.0;
      _write_struct.Tz = 0.0;
     #endif
	
	 // Scale and copy to _writeBuffer (a Wrench)
         // Measurements are in a right turning coordinate system (force exerted BY the sensor),
         // so switch Fy and Ty sign to get left turning measurement.
		 
         (*_writeBuffer)(0) =   (double)_write_struct.Fx * (double) _full_scale.Fx / 16384.0;
         (*_writeBuffer)(1) = - (double)_write_struct.Fy * (double) _full_scale.Fy / 16384.0;
         (*_writeBuffer)(2) =   (double)_write_struct.Fz * (double) _full_scale.Fz / 16384.0;
         // All the torques are in dNm (Nm*10), so scale:							        
	 (*_writeBuffer)(3) =   (double)_write_struct.Tx * (double) _full_scale.Tx / 16384.0 / 10;
         (*_writeBuffer)(4) = - (double)_write_struct.Ty * (double) _full_scale.Ty / 16384.0 / 10;         
	 (*_writeBuffer)(5) =   (double)_write_struct.Tz * (double) _full_scale.Tz / 16384.0 / 10;
						    
         outdatPort.Set( *_writeBuffer - _offset );
    }

    /**
     * This function is called when the task is stopped.
     */
    void WrenchSensor::shutdown() {


        }
   
    WrenchSensor::~WrenchSensor() {
        if(_writeBuffer)
            delete _writeBuffer;
    }




