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

#include "../RTallocCorbaToolkit.hpp"
#include "../../RTallocToolkit.hpp"

// use RTalloc RTT types::TypekitRepository test components
#include "../../tests/recv.hpp"

#include "taskBrowser/TaskBrowser.hpp"

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

	Recv			    recv("Recv");
    Activity	recv_activity(
        ORO_SCHED_OTHER, 0, 1.0 / 5, recv.engine());    // 5 Hz

	// Setup Corba and Export:
	ControlTaskServer::InitOrb(argc, argv);
	ControlTaskServer::Create( &recv );
	ControlTaskServer::ThreadOrb();

	// Wait for requests:
	recv.configure();
	recv_activity.start();
    OCL::TaskBrowser tb( &recv );
    tb.loop();
	recv_activity.stop();
      
	// Cleanup Corba:
    ControlTaskServer::ShutdownOrb();
	ControlTaskServer::DestroyOrb();

    destroy_memory_pool(rtMem);
    free(rtMem);

    return 0;
}
