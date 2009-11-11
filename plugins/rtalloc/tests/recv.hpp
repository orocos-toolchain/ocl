/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#ifndef	__RECV_HPP
#define	__RECV_HPP 1

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include "rtalloc/String.hpp"

// receive boost types from ports
class Recv : public RTT::TaskContext
{
public:
	RTT::DataPort<OCL::String>		        rtstring_port;

public:
	Recv(std::string name);
	virtual ~Recv();

    // no hooks, as we just want to see that the correct pattern is arriving
	// on the ports, and we do that using the taskbrowser in both the
    // deployer test and the 'combined' application test.
};

#endif
