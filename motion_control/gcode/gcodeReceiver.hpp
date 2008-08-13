// Copyright (C) 2006 Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef __GCODE_RECEIVER__
#define __GCODE_RECEIVER__

// RTT
#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

#include <kdl/frames.hpp>

namespace OCL
{

    class gcodeReceiver : public RTT::TaskContext
    {
    public:
        /// constructor
        gcodeReceiver(std::string name);

        /// destructor
        virtual~gcodeReceiver();

        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();

    private:
        // the properties of this task
        RTT::Property<int>          port_prop;
        RTT::Attribute<KDL::Frame>  initial_pos;

        // ports of this task
        RTT::DataPort<KDL::Frame>   cart_pos;

        // variables of the task
        KDL::Frame f;
        char       buffer[256];
        int        newsockfd;
        int        sockfd;
        double     data[6];
        bool       received;

    }; // class
} // namespace

#endif
