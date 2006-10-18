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

#ifndef __MYTASK1__
#define __MYTASK1__

// RTT
#include <rtt/RTT.hpp>
#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Commands.hpp>
#include <rtt/Methods.hpp>


namespace Template
{
    
  class Mytask1 : public RTT::GenericTaskContext
  {
  public:
      /// constructor
      Mytask1(std::string name, std::string propertyfilename="cpf/mytask1.cpf");

      /// destructor
      virtual~Mytask1();
    
      /// startup function is called once on start
      virtual bool startup();

      /// update function is called every time the activity triggers the task
      virtual void update();

      /// shutdown function is called once on shutdown
      virtual void shutdown();

  private:
      // the properties of this task
      RTT::Property<bool>         _my_property_1;
      RTT::Property<std::string>  _my_property_2;
      RTT::Property<int>          _my_property_3;
      RTT::Property<double>       _my_property_4;

      // methods of this task
      RTT::Method<void(void)>     _my_method_1;
      RTT::Method<bool(int)>      _my_method_2;

      // commands
      RTT::Command<bool(void)>    _my_command_1;

      // ports of this task
      RTT::ReadDataPort<double>   _my_readport_1;
      RTT::WriteDataPort<int>     _my_writeport_2;

      // variables of the task
      std::string                 _propertyfile;
      unsigned int                _my_var_1;
    
      // method functions
      void method1_function();
      bool method2_function(int a);
      
      // command functions
      bool command_function(int b);
      bool command_completion();


   }; // class
} // namespace

#endif
