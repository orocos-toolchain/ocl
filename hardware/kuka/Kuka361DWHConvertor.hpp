// Copyright  (C)  2008  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// Maintainer: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "Kuka361DWH.hpp"
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <ocl/OCL.hpp>

namespace OCL{

    class Kuka361DWHConvertor : public RTT::TaskContext
    {
    public:
        Kuka361DWHConvertor(const std::string& name);
        ~Kuka361DWHConvertor(){};
        
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook(){};
        
    private:
        std::vector<double> naxes_positions_local, geometric_positions_local,
            naxes_velocities_local, geometric_velocities_local;
        
        RTT::DataPort<std::vector<double> > naxes_positions, geometric_positions,
            naxes_velocities, geometric_velocities;
    };
}
