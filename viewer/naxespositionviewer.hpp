#ifndef OROCOS_COMP_NAXESPOSITIONVIEWER_HPP
#define OROCOS_COMP_NAXESPOSITIONVIEWER_HPP
#include <vector>
#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>




#include <ocl/OCL.hpp>

namespace OCL {
    /**
     * This class implements a TaskContext that sends position values
     * to the KDLViewer using an ACE-socket.
     *
     * Singlethreaded reactive server that can handle multiple clients.
     */

class NAxesPositionViewer : public RTT::TaskContext
{
public:
    /**
     * Constructer of this class
     *
     * @param name name of the TaskContext
     * @param propertyfilename location of the propertyfile. Default: cpf/viewer.cpf
     *
     */
  NAxesPositionViewer(const std::string& name,const std::string& propertyfilename="cpf/viewer.cpf");
  virtual ~NAxesPositionViewer();

private:
  /**
   * A local copy of the name of the propertyfile so we can store changed
   * properties.
   */
 const std::string _propertyfile;

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

protected:
    /// number of the port that the ACE-socket should use
    RTT::Property<int>               		           portnumber;
    /// number of axes to view
    RTT::Property<int>                                  num_axes;
    /// boolean whether the positionvalues are stored in separte ports
    /// or in one port which contains a vector of values
    RTT::Property<bool>                                 seperate_ports;
    /// name of the port (if seperate ports NAxesPositionViewer will
    /// look for ports port_name0, port_name1, ...
    RTT::Property<std::string>                          port_name;
private:
    int                                                 _num_axes;
protected:
    /// Vector of dataports containing the position values (used when
    /// NAxesPositionViewer::seperate_ports is true)
    std::vector<RTT::ReadDataPort<double>*>             seperateValues;
    /// Dataports containing the vector of position values (used when
    /// NAxesPositionViewer::seperate_ports is false)
    RTT::ReadDataPort<std::vector<double> >*            vectorValue;
private:
   std::vector<double>                                 jointvec;
   void*                                     		   clientacceptor;
   int												   state;
			// 0 = stopped, 1 = want to startup, 2 = running
};

} // namespace Orocos

#endif // NAXESVELOCITYVIEWER
