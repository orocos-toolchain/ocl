// Copyright  (C)  2008  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Version: 1.0
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

#include <rtt/types/TypekitPlugin.hpp>
#include <rtt/types/SequenceTypeInfo.hpp>

#include <string>
#include <vector>

namespace OCL
{
    using namespace RTT;
    using namespace RTT::detail;
    using namespace std;

    class OCLTypekit : public TypekitPlugin
    {
    public:
        bool loadTypes() {
            RTT::types::TypeInfoRepository::Instance()->addType( new types::SequenceTypeInfo<vector<std::string> >("strings") );

            // segfaults when reading out an element of this type:
            //RTT::types::TypeInfoRepository::Instance()->addType( new types::SequenceTypeInfo<vector<bool> >("bools") );

            RTT::types::TypeInfoRepository::Instance()->addType( new types::SequenceTypeInfo<vector<double> >("doubles") );

            RTT::types::TypeInfoRepository::Instance()->addType( new types::SequenceTypeInfo<vector<int> >("ints") );

            return true;
        }

        bool loadOperators() { return true; }
        bool loadConstructors() { return true; }

        std::string getName() {
             return "OCLTypekit";
        }
    };
}

ORO_TYPEKIT_PLUGIN( OCL::OCLTypekit )

