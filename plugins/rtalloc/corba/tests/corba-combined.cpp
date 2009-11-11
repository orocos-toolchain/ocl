/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)

 Test components within the same executable.
 ***************************************************************************/
#include <signal.h>

#include <rtt/os/main.h>
#include <rtt/corba/ControlTaskServer.hpp>
#include <rtt/corba/ControlTaskProxy.hpp>
#include <rtt/RTT.hpp>
#include <rtt/PeriodicActivity.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/os/main.h>
#include <rtt/Ports.hpp>

// need access to all TLSF functions embedded in RTT
#define ORO_MEMORY_POOL
#include <rtt/os/tlsf/tlsf.h>

// use RTalloc RTT Toolkit test components
#include "../../tests/send.hpp"
#include "../../tests/recv.hpp"

using namespace std;
using namespace Orocos;
using namespace RTT::Corba;


void sighandler(int, siginfo_t*, void*)
{
	ControlTaskServer::ShutdownOrb(false);
}

int ORO_main(int argc, char* argv[])
{
    // initialize TLSF
    static const size_t RT_MEM_SIZE = 20*1024;  // 20 kb should do  
    void* rtMem     = malloc(RT_MEM_SIZE);
    assert(rtMem);
    size_t freeMem  = init_memory_pool(RT_MEM_SIZE, rtMem);
    assert(-1 != freeMem);
    freeMem = freeMem;      // avoid compiler warning

	Recv		        recv("Recv");
    PeriodicActivity	recv_activity(
		ORO_SCHED_OTHER, 0, 0.1, recv.engine());
	Send     		    send("Send");
    PeriodicActivity	send_activity(
		ORO_SCHED_OTHER, 0, 0.2, send.engine());

	// Setup Corba, create a server and then a proxy for the send
    // component, and then connect recv to the proxy
	ControlTaskServer::InitOrb(argc, argv);
	ControlTaskServer::Create( &send );

	TaskContext* proxy = ControlTaskProxy::Create( "Send" );
	assert(NULL != proxy);
	if ( connectPeers( proxy, &recv ) == false )
	{
		log(Error) << "Could not connect peers !"<<endlog();
	}
	// create data object at recv's side
	if ( connectPorts( proxy, &recv) == false )
	{
		log(Error) << "Could not connect ports !"<<endlog();
	}

	// Wait for requests:
	send.configure();
	recv.configure();
	send_activity.start();
	recv_activity.start();

	// needs Ctrl-C to quit
	struct sigaction	sa;
	struct sigaction	old_sa;
	memset(&sa, 0, sizeof(sa));
	memset(&old_sa, 0, sizeof(old_sa));
	sa.sa_sigaction	= sighandler;
	sa.sa_flags		= SA_SIGINFO;
	if (0 != sigaction(SIGINT, &sa, &old_sa))
	{
		log(Error) << "Error connecting signal handler" << endlog();
		return -1;
	}
	
	ControlTaskServer::RunOrb();

	send_activity.stop();
	recv_activity.stop();

	// Cleanup Corba:
//	ControlTaskServer::ShutdownOrb(false);

	(void)sigaction(SIGINT, &old_sa, NULL);

    destroy_memory_pool(rtMem);
    free(rtMem);

    return 0;
}
