#ifndef OROCOS_COMP_NAXESPOSITIONVIEWER_HPP
#define OROCOS_COMP_NAXESPOSITIONVIEWER_HPP
#include <vector>
#include <corelib/RTT.hpp>

#include <execution/GenericTaskContext.hpp>
#include <execution/Ports.hpp>
//#include <corelib/Event.hpp>
#include <corelib/Properties.hpp>




namespace Orocos {

class NAxesPositionViewer : public RTT::GenericTaskContext
{
public:
  NAxesPositionViewer(const std::string& name,const std::string& propertyfilename="cpf/viewer.cpf");
  virtual ~NAxesPositionViewer();

protected:  
  /**
   * A local copy of the name of the propertyfile so we can store changed
   * properties.
   */
 const std::string _propertyfile;

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

   
   RTT::Property<int>               		           portnumber;
   RTT::Property<int>                                  num_axes;
   int                                                 _num_axes;
   std::vector<RTT::ReadDataPort<double>*>             positionValue;
   std::vector<double>                                 jointvec;
   void*                                     		   clientacceptor;
   int												   state;
			// 0 = stopped, 1 = want to startup, 2 = running
};

} // namespace Orocos

#endif // NAXESVELOCITYVIEWER 
