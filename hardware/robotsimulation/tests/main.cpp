// Copyright (C) 2007 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
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

#include <hardware/robotsimulation/nAxesVelocityController.hpp>

#include <taskbrowser/TaskBrowser.hpp>
#include <reporting/FileReporting.hpp>

#include <rtt/os/main.h>
#include <rtt/PeriodicActivity.hpp>
#include <rtt/Attribute.hpp>

using namespace Orocos;
using namespace RTT;
using namespace std;

int ORO_main(int argc, char* argv[])
{
    int axes = 6;
    if (argc>1)
        axes=atoi(argv[1]);
        
    nAxesVelocityController myMachine("MyNAxesMachine",axes);
    
    Attribute<vector<double> > velocities("velocities",vector<double>(6,0));
    myMachine.attributes()->addAttribute(&velocities);
        
    TaskBrowser browser(&myMachine);
    browser.setColorTheme(TaskBrowser::whitebg);
    
    PeriodicActivity myMachineTask(ORO_SCHED_OTHER,0,0.01,myMachine.engine());
    
    browser.loop();
    return 0;
}
