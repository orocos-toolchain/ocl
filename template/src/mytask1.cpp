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

#include "task1.hpp"

using namespace std;
using namespace RTT;
  
Mytask1::Mytask1(string name, string propertyfile):
    GenericTaskContext(name),
    // name the properties
    _my_property_1("propname1","discription of property 1"),
    _my_property_2("propname2","discription of property 2"),
    _my_property_3("propname3","discription of property 3"),
    _my_property_4("propname4","discription of property 4"),
    // name the methods
    _my_method_1("methodname1", &Mytask1::method1_function, this),
    _my_method_2("methodname2", &Mytask1::method2_function, this),
    // name the commands
    _my_command_1("commandname1", &Mytask1::command_function, &Mytask1::command_completion, this),
    // name the ports
    _my_readport_1("portname1"),
    _my_writeport_2("portname2"),
    // initialize variables
    _propertyfile(propertyfile)
{
    // register and read properties
    log(Info) << this->getName() << ": registring and reading Properties" << endlog();
    properties()->addProperty(&_my_property_1);
    properties()->addProperty(&_my_property_2);
    properties()->addProperty(&_my_property_3);
    properties()->addProperty(&_my_property_4);
    if (!marshalling()->readAllProperties(_propertyfile)) log(Error) << "Reading properties failed." << endlog();
    
    // register methods
    log(Info) << this->getName() << ": registring methods" << endlog();
    methods()->addMethod(&_my_method_1, "description of method 1");
    methods()->addMethod(&_my_method_2, "description of method 2", "parameter1", "description of parameter 1");
    
    // register commands
    log(Info) << this->getName() << ": registring commands" << endlog();
    commands()->addCommand(&_my_command_1, "description of command 1");
    
    // register ports
    log(Info) << this->getName() << ": registring ports" << endlog();
    ports()->addPort(&_my_readport_1);
    ports()->addPort(&_my_writeport_2);
}


Mytask1::~Mytask1()
{
}


bool Mytask1::startup()    
{
    log(Info) << this->getName() << ": started" << endlog();
    return true;
}


void Mytask1::update()
{
    log(Info) << this->getName() << ": update" << endlog();
}


void Mytask1::shutdown()
{
    log(Info) << this->getName() << ": shutdown" << endlog();
    writeProperties(_propertyfile);
}


void Mytask1::method1_function()
{
    log(Info) << this->getName() << ": method 1" << endlog();
}


bool Mytask1::method2_function(int a)
{
    log(Info) << this->getName() << ": method 2 with parameter = " << a << endlog();
    return true;
}


bool command_function(int b)
{
    log(Info) << this->getName() << ": command 1 with parameter = " << b << endlog();
    return true;
}


bool command_completion()
{
    return true;
}


