/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include "recv.hpp"

Recv::Recv(std::string name) :
		RTT::TaskContext(name),
		rtstring_port("rtstring")
{
	ports()->addPort( rtstring_port );
}

Recv::~Recv()
{
}
