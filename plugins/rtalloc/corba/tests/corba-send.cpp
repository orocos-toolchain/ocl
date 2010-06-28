/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/

#include <rtt/corba/ControlTaskServer.hpp>
#include <rtt/corba/ControlTaskProxy.hpp>
#include <rtt/RTT.hpp>
#include <rtt/Activity.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>
#include <rtt/Port.hpp>

// need access to all TLSF functions embedded in RTT
#define ORO_MEMORY_POOL
#include <rtt/os/tlsf/tlsf.h>

#include "taskBrowser/TaskBrowser.hpp"

#include "../RTallocCorbaToolkit.hpp"
#include "../../RTallocToolkit.hpp"

// use RTalloc RTT types::TypekitRepository test components
#include "../../tests/send.hpp"

using namespace std;
using namespace Orocos;
using namespace RTT::corba;

int ORO_main(int argc, char* argv[])
{
    // initialize TLSF
    static const size_t RT_MEM_SIZE = 20*1024;  // 20 kb should do  
    void* rtMem     = malloc(RT_MEM_SIZE);
    assert(rtMem);
    size_t freeMem  = init_memory_pool(RT_MEM_SIZE, rtMem);
    assert((size_t)-1 != freeMem);
    freeMem = freeMem;      // avoid compiler warning

    RTT::types::TypekitRepository::Import( RTT::RTallocToolkit  );
    RTT::types::TypekitRepository::Import( RTT::corba::corbaRTallocPlugin  );

	Send				send("Send");
    Activity	send_activity(
		ORO_SCHED_OTHER, 0, 1.0 / 10, send.engine());   // 10 Hz

	// start Corba and find the remote task
	ControlTaskProxy::InitOrb(argc, argv);
	ControlTaskServer::ThreadOrb();
	TaskContext* recv = ControlTaskProxy::Create( "Recv" );
	assert(NULL != recv);

	if ( connectPeers( recv, &send ) == false )
	{
		log(Error) << "Could not connect peers !"<<endlog();
	}
	// create data object at recv's side
	if ( connectPorts( recv, &send) == false )
	{
		log(Error) << "Could not connect ports !"<<endlog();
	}

    // run for 5 seconds
	send.configure();
	send_activity.start();
	log(Info) << "Starting task browser" << endlog();
	OCL::TaskBrowser tb( recv );
	tb.loop();
	send_activity.stop();

	ControlTaskProxy::DestroyOrb();

    destroy_memory_pool(rtMem);
    free(rtMem);

    return 0;
}

