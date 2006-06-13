// hardware interfaces

#include "Kuka160nAxesVelocityController.hpp"

//User interface
#include "../../taskbrowser/TaskBrowser.hpp"

//main
//#include <corelib/Activities.hpp>
//#include <execution/GenericTaskContext.hpp>
//#include <corelib/Logger.hpp>
#include <os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace std;

class EmergencyStop
{
public:
  EmergencyStop(GenericTaskContext *axes)
    : _axes(axes), _fired(false)  {
    _stop = _axes->commands()->create("this", "stopAllAxes");
    _lock = _axes->commands()->create("this", "lockAllAxes");
  };
  ~EmergencyStop(){};
  void Handle() {
    if (!_fired){
	_stop.execute();
	_lock.execute();
    }
  };
  void Complete(){
    if (!_fired){ _fired = true;
    cout << "---------------------------------------------" << endl;
    cout << "--------- EMERGENCY STOP --------------------" << endl;
    cout << "---------------------------------------------" << endl; }};
private:
  GenericTaskContext *_axes;
  bool _fired;
  CommandC _stop;
  CommandC _lock;
}; // class


using namespace Orocos;
using namespace RTT;
using namespace std;


/**
 * main() function
 */
int ORO_main(int argc, char* argv[])
{

  if ( Logger::log().getLogLevel() < Logger::Info ) {
    Logger::log().setLogLevel( Logger::Info );
              Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
  }
  
  RTT::GenericTaskContext* my_robot = new Kuka160nAxesVelocityController();
  
  NonPreemptibleActivity _kukaTask(1, my_robot->engine() );  // very slow for debugging purposes
  
  TaskBrowser browser( my_robot );
  
  browser.setColorTheme( TaskBrowser::whitebg );
  
  /**
   * Start the console reader.
   */
  browser.loop();
  
  /**
   * End of C++ examples, start TaskBrowser to let user play:
   */
  
  _kukaTask.stop();

  delete my_robot;
    
  return 0;
}
