/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#ifndef	__SEND_HPP
#define	__SEND_HPP 1

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include "rtalloc/String.hpp"

// send RTalloc types on ports
class Send : public RTT::TaskContext
{
public:
	RTT::DataPort<OCL::String>		        rtstring_port;

public:
	Send(std::string name);
	virtual ~Send();

	virtual void updateHook();
};

#endif
