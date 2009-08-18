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

#include <rtt/os/StartStopManager.hpp>
#include <rtt/types/TemplateTypeInfo.hpp>
#include <rtt/types/Operators.hpp>
#include <rtt/types/OperatorTypes.hpp>
#include <rtt/types/RealTimeToolkit.hpp>

#include <rtt/types/VectorTemplateComposition.hpp>

namespace OCL
{
    using namespace RTT;
    using namespace RTT::detail;

    int loadOCL()
    {
        //RTT::types::TypeInfoRepository::Instance()->addType( new types::StdVectorTemplateTypeInfo<std::string>("stringList") );
        RTT::types::TypeInfoRepository::Instance()->addType( new types::StdVectorTemplateTypeInfo<std::string,true>("strings") );
        RTT::types::TypeInfoRepository::Instance()->type("strings")->addConstructor(newConstructor(types::stdvector_ctor<std::string>() ) );
        RTT::types::TypeInfoRepository::Instance()->type("strings")->addConstructor(newConstructor(types::stdvector_ctor2<std::string>() ) );
        RTT::types::TypeInfoRepository::Instance()->type("strings")->addConstructor(new types::StdVectorBuilder<std::string>() );
        RTT::types::OperatorRepository::Instance()->add( newBinaryOperator( "[]", types::stdvector_index<std::string>() ) );
        RTT::types::OperatorRepository::Instance()->add( newDotOperator( "size", types::get_size<const std::vector<std::string>&>() ) );

        RTT::types::TypeInfoRepository::Instance()->addType( new types::StdVectorTemplateTypeInfo<bool,true>("bools") );
        RTT::types::TypeInfoRepository::Instance()->type("bools")->addConstructor(newConstructor(types::stdvector_ctor<bool>() ) );
        RTT::types::TypeInfoRepository::Instance()->type("bools")->addConstructor(newConstructor(types::stdvector_ctor2<bool>() ) );
        RTT::types::TypeInfoRepository::Instance()->type("bools")->addConstructor(new types::StdVectorBuilder<bool>() );
        RTT::types::OperatorRepository::Instance()->add( newBinaryOperator( "[]", types::stdvector_index<bool>() ) );
        RTT::types::OperatorRepository::Instance()->add( newDotOperator( "size", types::get_size<const std::vector<bool>&>() ) );

//        RTT::types::TypeInfoRepository::Instance()->addType( new types::StdVectorTemplateTypeInfo<double,true>("doubles") );
//        RTT::types::TypeInfoRepository::Instance()->type("doubles")->addConstructor(newConstructor(types::stdvector_ctor<double>() ) );
//        RTT::types::TypeInfoRepository::Instance()->type("doubles")->addConstructor(newConstructor(types::stdvector_ctor2<double>() ) );
//        RTT::types::TypeInfoRepository::Instance()->type("doubles")->addConstructor(new types::StdVectorBuilder<double>() );
//        RTT::types::OperatorRepository::Instance()->add( newBinaryOperator( "[]", types::stdvector_index<double>() ) );

        RTT::types::TypeInfoRepository::Instance()->addType( new types::StdVectorTemplateTypeInfo<int,true>("ints") );
        RTT::types::TypeInfoRepository::Instance()->type("ints")->addConstructor(newConstructor(types::stdvector_ctor<int>() ) );
        RTT::types::TypeInfoRepository::Instance()->type("ints")->addConstructor(newConstructor(types::stdvector_ctor2<int>() ) );
        RTT::types::TypeInfoRepository::Instance()->type("ints")->addConstructor(new types::StdVectorBuilder<int>() );
        RTT::types::OperatorRepository::Instance()->add( newBinaryOperator( "[]", types::stdvector_index<int>() ) );
        RTT::types::OperatorRepository::Instance()->add( newDotOperator( "size", types::get_size<const std::vector<int>&>() ) );
        
        return true;
    }

    os::InitFunction OCLLoader(&loadOCL);
}


