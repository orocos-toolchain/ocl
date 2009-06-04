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
#include <rtt/TemplateTypeInfo.hpp>
#include <rtt/Operators.hpp>
#include <rtt/OperatorTypes.hpp>
#include <rtt/RealTimeToolkit.hpp>

#include <rtt/VectorTemplateComposition.hpp>

namespace OCL
{
    using namespace RTT;
    using namespace RTT::detail;

    int loadOCL()
    {
        //RTT::TypeInfoRepository::Instance()->addType( new StdVectorTemplateTypeInfo<std::string>("stringList") );
        RTT::TypeInfoRepository::Instance()->addType( new StdVectorTemplateTypeInfo<std::string,true>("strings") );
        RTT::TypeInfoRepository::Instance()->type("strings")->addConstructor(newConstructor(stdvector_ctor<std::string>() ) );
        RTT::TypeInfoRepository::Instance()->type("strings")->addConstructor(newConstructor(stdvector_ctor2<std::string>() ) );
        RTT::TypeInfoRepository::Instance()->type("strings")->addConstructor(new StdVectorBuilder<std::string>() );
        RTT::OperatorRepository::Instance()->add( newBinaryOperator( "[]", stdvector_index<std::string>() ) );
        RTT::OperatorRepository::Instance()->add( newDotOperator( "size", get_size<const std::vector<std::string>&>() ) );

        RTT::TypeInfoRepository::Instance()->addType( new StdVectorTemplateTypeInfo<bool,true>("bools") );
        RTT::TypeInfoRepository::Instance()->type("bools")->addConstructor(newConstructor(stdvector_ctor<bool>() ) );
        RTT::TypeInfoRepository::Instance()->type("bools")->addConstructor(newConstructor(stdvector_ctor2<bool>() ) );
        RTT::TypeInfoRepository::Instance()->type("bools")->addConstructor(new StdVectorBuilder<bool>() );
        RTT::OperatorRepository::Instance()->add( newBinaryOperator( "[]", stdvector_index<bool>() ) );
        RTT::OperatorRepository::Instance()->add( newDotOperator( "size", get_size<const std::vector<bool>&>() ) );

//        RTT::TypeInfoRepository::Instance()->addType( new StdVectorTemplateTypeInfo<double,true>("doubles") );
//        RTT::TypeInfoRepository::Instance()->type("doubles")->addConstructor(newConstructor(stdvector_ctor<double>() ) );
//        RTT::TypeInfoRepository::Instance()->type("doubles")->addConstructor(newConstructor(stdvector_ctor2<double>() ) );
//        RTT::TypeInfoRepository::Instance()->type("doubles")->addConstructor(new StdVectorBuilder<double>() );
//        RTT::OperatorRepository::Instance()->add( newBinaryOperator( "[]", stdvector_index<double>() ) );

        RTT::TypeInfoRepository::Instance()->addType( new StdVectorTemplateTypeInfo<int,true>("ints") );
        RTT::TypeInfoRepository::Instance()->type("ints")->addConstructor(newConstructor(stdvector_ctor<int>() ) );
        RTT::TypeInfoRepository::Instance()->type("ints")->addConstructor(newConstructor(stdvector_ctor2<int>() ) );
        RTT::TypeInfoRepository::Instance()->type("ints")->addConstructor(new StdVectorBuilder<int>() );
        RTT::OperatorRepository::Instance()->add( newBinaryOperator( "[]", stdvector_index<int>() ) );
        RTT::OperatorRepository::Instance()->add( newDotOperator( "size", get_size<const std::vector<int>&>() ) );
        
        return true;
    }

    OS::InitFunction OCLLoader(&loadOCL);
}


