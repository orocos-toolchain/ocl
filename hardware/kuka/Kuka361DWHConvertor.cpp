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

#include "Kuka361DWHConvertor.hpp"
#include <ocl/ComponentLoader.hpp>

ORO_LIST_COMPONENT_TYPE( OCL::Kuka361DWHConvertor )

namespace OCL{
    using namespace std;
    using namespace RTT;

    Kuka361DWHConvertor::Kuka361DWHConvertor(const std::string& name):
        TaskContext(name,Stopped),
        naxes_positions_local(6,0.0),
        geometric_positions_local(6,0.0),
        naxes_velocities_local(6,0.0),
        geometric_velocities_local(6,0.0),
        naxes_positions("nAxesSensorPosition",naxes_positions_local),
        geometric_positions("GeometricSensorPosition",geometric_positions_local),
        naxes_velocities("nAxesOutputVelocity",naxes_velocities_local),
        geometric_velocities("GeometricOutputVelocity",geometric_velocities_local)
    {
        ports()->addPort(&naxes_positions);
        ports()->addPort(&geometric_positions);
        ports()->addPort(&naxes_velocities);
        ports()->addPort(&geometric_velocities);
    }
    
    void Kuka361DWHConvertor::updateHook()
    {
        if((naxes_positions.Get().size()==6)&&
           (geometric_velocities.Get().size()==6)){
            Kuka361DWH::convertGeometric(naxes_positions.Get(),geometric_velocities.Get(),
                                         geometric_positions_local,naxes_velocities_local);
            naxes_velocities.Set(naxes_velocities_local);
            geometric_positions.Set(geometric_positions_local);
        }else{
            naxes_velocities.Set(vector<double>(6,0.0));
            geometric_positions.Set(vector<double>(6,0.0));
            this->error();
        }
    }
    
    
    void Kuka361DWHConvertor::errorHook()
    {
        if((naxes_positions.Get().size()==6)&&
           (geometric_velocities.Get().size()==6)){
            this->recovered();
        }
    }
    
}


    
