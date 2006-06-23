//hardware interfaces

#include "CaptureCamera.hpp"
//#include "ShowImages.hpp"

//User interface
#include "../../taskbrowser/TaskBrowser.hpp"

#include <os/main.h>

using namespace Orocos;
using namespace RTT;
using namespace std;

// main() function

int ORO_main(int argc, char* argv[])
{
  
  CaptureCamera camera("Camera");
  //ShowImages show("Images");
    
  //show.connectPeers(&camera);
  
  /// Link my_robot to Taskbrowser
  TaskBrowser browser(&camera);
  browser.setColorTheme( TaskBrowser::whitebg );

  /// Creating Tasks
  NonPreemptibleActivity cameraTask(0.02, camera.engine() );  
  
  /// Start the console reader.
  browser.loop();
  
    return 0;
}
