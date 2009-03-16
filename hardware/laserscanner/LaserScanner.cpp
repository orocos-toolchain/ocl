// Copyright (C) 2006 new Meeussen <new dot meeussen at mech dot kuleuven dot be>
//               2008 Ruben Smits < first dot last at mech etc>
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

#include "LaserScanner.hpp"
#include "SickLMS200.h"
#include <rtt/Logger.hpp>
#include <iostream>
#include <ocl/ComponentLoader.hpp>

ORO_CREATE_COMPONENT( OCL::LaserScanner );

namespace OCL
{
    using namespace RTT;
    using namespace std;
    using namespace SickDriver;

    LaserScanner::LaserScanner(string name):
        TaskContext(name),
        port("port","Serial port to which the Sick scanner is connected. (/dev/ttySx)","/dev/ttyS0"),
        range_mode("range_mode","Should be 100 or 180 degrees",180),
        res_mode("res_mode","Should be 0.25,0.5,1 degrees",0.5),
        unit_mode("unit_mode","Should be mm or cm","mm"),
        distances("LaserDistance"),
        angles("LaserAngle"),
        distanceOutOfRange("distanceOutOfRange")
    {
        log(Debug) <<this->TaskContext::getName()<<": adding Properties"<<endlog();
        properties()->addProperty(&port);
        properties()->addProperty(&range_mode);
        properties()->addProperty(&res_mode);
        properties()->addProperty(&unit_mode);

        log(Debug) <<this->TaskContext::getName()<<": adding Ports"<<endlog();
        ports()->addPort(&distances);
        ports()->addPort(&angles);

        log(Debug) <<this->TaskContext::getName()<<": adding Events"<<endlog();
        events()->addEvent(&distanceOutOfRange, "Distance out of Range", "C", "Channel", "V", "Value");

        log(Debug) <<this->TaskContext::getName()<<": constructed."<<endlog();
        buf = new unsigned char[MAXNDATA];
    }


    LaserScanner::~LaserScanner()
    {
        sick_laserscanner->stop();
        delete buf;
        delete sick_laserscanner;
    }

    bool LaserScanner::configureHook()
    {
        Logger::In in(this->TaskContext::getName());

        log(Debug) <<this->TaskContext::getName()<<": create Sick laserscanner"<<endlog();
        port_char = port.value().c_str();

        if (range_mode.value() == 100 )
            range_mode_char = SickLMS200::RANGE_100;
        else if (range_mode.value() == 180 )
            range_mode_char = SickLMS200::RANGE_180;
        else {
            log(Error) <<"Wrong range parameter. Should be 100 or 180."<<endlog();
            return false;
        }

        if (res_mode.value() == 1.0 )
            res_mode_char = SickLMS200::RES_1_DEG;
        else if (res_mode.value() == 0.5 )
            res_mode_char = SickLMS200::RES_0_5_DEG;
        else if (res_mode.value() == 0.25 && range_mode.value() == 100 )
            res_mode_char = SickLMS200::RES_0_25_DEG;
        else if (res_mode.value() == 0.25 && range_mode.value() == 180 ) {
            log(Error) <<" Res 0.25 does not work with range mode 180."<<endlog();
            return false;
        } else{
            log(Error) <<"Wrong res_mode parameter. Should be 0.25 or 0.5 or 1.0."<<endlog();
            return false;
        }

        if (unit_mode.value() == "cm" )
            unit_mode_char = SickLMS200::CMMODE;
        else if (unit_mode.value() == "mm" )
            unit_mode_char = SickLMS200::MMMODE;
        else {
            log(Error)<<"Wrong unit mode parameter. Should be cm or mm."<<endlog();
            return false;
        }

        num_meas = (unsigned int)(range_mode.value()/res_mode.value())+1;
        distances_local.resize(num_meas);
        angles_local.resize(num_meas);
        for (unsigned int i=0; i<num_meas; i++)
            angles_local[i] = ((double)i/((double)num_meas-1.0))*(double)(range_mode.value()*M_PI/180);

        sick_laserscanner = new SickLMS200(port_char, range_mode_char, res_mode_char, unit_mode_char);
        return true;
    }


    bool LaserScanner::startHook()
    {
        Logger::In in(this->TaskContext::getName());
        if (!sick_laserscanner->start()){
            log(Error)<<"Starting laserscanner failed."<<endlog();
            return false;
        }

        if (!registerSickLMS200SignalHandler()){
            log(Error)<<"Register Sick signal handler failed."<<endlog();
            return false;
        }

        angles.Set(angles_local);

        return true;
    }


    void LaserScanner::updateHook()
    {
        sick_laserscanner->readMeasurement(buf,datalen);
        
        if (sick_laserscanner->checkErrorMeasurement()){
            log(Error)<<"Measurement error."<<endlog();
            error();
        }
        
        if (!sick_laserscanner->checkPlausible()){
            log(Warning)<<"Measurement not reliable."<<endlog();
            warning();
        }
        
        if (num_meas*2 != (unsigned int)datalen){
            log(Error)<<"Number of measurements inconsistent: expected "
                      << num_meas << " and received " << datalen/2 <<endlog();
            warning();
        }

        for (int i=0; i<datalen; i=i+2)
            distances_local[(unsigned int)(i/2)] = ((double)( (buf[i+1] & 0x1f) <<8  |buf[i]))/1000.0;
        distances.Set(distances_local);
        
        this->engine()->getActivity()->trigger();
    }

    void LaserScanner::stopHook()
    {
        if (!sick_laserscanner->stop())
            log(Error)<<"Error stopping laserscanner"<<endlog();
    }
}//namespace

