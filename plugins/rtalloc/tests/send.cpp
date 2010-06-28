/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include "send.hpp"
#include <rtt/Logger.hpp>
using namespace RTT;

Send::Send(std::string name) :
		RTT::TaskContext(name),
		rtstring_port("rtstring")
{
	ports()->addPort( rtstring_port );
}

Send::~Send()
{
}

void Send::updateHook()
{
    static int i =0;
    
    char    str[100];
    snprintf(&str[0], sizeof(str),
             "rtstring %d", i);
    
    OCL::String r(&str[0]);
//    log(Error) << "r=" << r.c_str() << endlog();  
    rtstring_port.write( r );
    
    ++i;
}
