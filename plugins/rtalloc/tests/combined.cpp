/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)

 Test components that are directly connected within the same executable.
 ***************************************************************************/
#include <assert.h>
#include <rtt/RTT.hpp>
#include <rtt/PeriodicActivity.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>
#include <rtt/Ports.hpp>
// need access to all TLSF functions embedded in RTT
#define ORO_MEMORY_POOL
#include <rtt/os/tlsf/tlsf.h>
#include "taskBrowser/TaskBrowser.hpp"

#include "send.hpp"
#include "recv.hpp"
#include "../RTallocToolkit.hpp"

using namespace std;
using namespace Orocos;

int ORO_main(int argc, char* argv[])
{
    // initialize TLSF
    static const size_t RT_MEM_SIZE = 20*1024;  // 20 kb should do  
    void* rtMem     = malloc(RT_MEM_SIZE);
    assert(rtMem);
    size_t freeMem  = init_memory_pool(RT_MEM_SIZE, rtMem);
    assert((size_t)-1 != freeMem);
    freeMem = freeMem;      // avoid compiler warning

    // forcibly load the toolkit
    RTT::Toolkit::Import(RTT::RTallocToolkit);
	
	Recv		        recv("Recv");
    PeriodicActivity	recv_activity(
		ORO_SCHED_OTHER, 0, 0.1, recv.engine());
	Send     		send("Send");
    PeriodicActivity	send_activity(
		ORO_SCHED_OTHER, 0, 0.2, send.engine());

    // connect the two components 
	if ( connectPeers( &send, &recv ) == false )
	{
		log(Error) << "Could not connect peers !"<<endlog();
	}
	if ( connectPorts( &send, &recv) == false )
	{
		log(Error) << "Could not connect ports !"<<endlog();
	}

	send.configure();
	recv.configure();
	send_activity.start();
	recv_activity.start();

    // task browser
    TaskBrowser browser( &recv );
    browser.setColorTheme( TaskBrowser::whitebg );
    browser.loop();

	send_activity.stop();
	recv_activity.stop();

    destroy_memory_pool(rtMem);
    free(rtMem);

    return 0;
}
