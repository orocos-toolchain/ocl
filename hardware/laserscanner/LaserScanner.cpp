// Copyright (C) 2006 new Meeussen <new dot meeussen at mech dot kuleuven dot be>
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

#include "LaserScanner.hpp"
#include <iostream>




namespace OCL
{
  using namespace RTT;
  using namespace std;
  using namespace SickDriver;
  
  LaserScanner::LaserScanner(string name, string propertyfile):
    TaskContext(name),
    _port("port",""),
    _range_mode("range_mode",""),
    _res_mode("res_mode",""),
    _unit_mode("unit_mode",""),
    _distances("LaserDistance"),
    _distanceOutOfRange("distanceOutOfRange"),
    _propertyfile(propertyfile)
  {
    log(Debug) <<this->getName()<<": adding Properties"<<endlog();
    properties()->addProperty(&_port);
    properties()->addProperty(&_range_mode);
    properties()->addProperty(&_res_mode);
    properties()->addProperty(&_unit_mode);

    if(!marshalling()->readProperties(_propertyfile))
      log(Error)<<"Reading properties failed."<<endlog();

    log(Debug) <<this->getName()<<": adding Ports"<<endlog();
    ports()->addPort(&_distances);

    log(Debug) <<this->getName()<<": adding Events"<<endlog();
    events()->addEvent(&_distanceOutOfRange, "Distance out of Range", "C", "Channel", "V", "Value");
    
    log(Debug) <<this->getName()<<": create Sick laserscanner"<<endlog();
    if (_port.value() == 0 ) _port_char = "/dev/ttyS0";
    else log(Error) << this->getName()<<"Wrong port parameter. Should be 0."<<endlog();

    if (_range_mode.value() == 100 ) _range_mode_char = SickLMS200::RANGE_100;
    else if (_range_mode.value() == 180 ) _range_mode_char = SickLMS200::RANGE_180;
    else log(Error) << this->getName()<<"Wrong range parameter. Should be 100 or 180."<<endlog();
    
    if (_res_mode.value() == 0.25 ) _res_mode_char = SickLMS200::RES_0_25_DEG;
    else if (_res_mode.value() == 0.5 ) _res_mode_char = SickLMS200::RES_0_5_DEG;
    else if (_res_mode.value() == 1.0 ) _res_mode_char = SickLMS200::RES_1_DEG;
    else log(Error) << this->getName()<<"Wrong res_mode parameter. Should be 0.25 or 0.5 or 1.0."<<endlog();

    if (_unit_mode.value() == "cm" ) _unit_mode_char = SickLMS200::CMMODE;
    else if (_unit_mode.value() == "mm" ) _unit_mode_char = SickLMS200::MMMODE;
    else log(Error) << this->getName()<<"Wrong unit mode parameter. Should be cm or mm."<<endlog();

    _sick_laserscanner = new SickLMS200(_port_char, _range_mode_char, _res_mode_char, _unit_mode_char);

    log(Debug) <<this->getName()<<": constructed."<<endlog();
  }

  
  LaserScanner::~LaserScanner()
  {
      delete _sick_laserscanner;
  }
  

  bool LaserScanner::startup()    
  {
      _sick_laserscanner->start();
      registerSickLMS200SignalHandler();
      return true;
  }
    

  void LaserScanner::update()
  {
      uchar buf[MAXNDATA];
      int datalen;
      _sick_laserscanner->readMeasurement(buf,datalen);
      if (_sick_laserscanner->checkErrorMeasurement())
          log(Error)<<this->getName()<<": measurement error."<<endlog();
      if (!_sick_laserscanner->checkPlausible()) 
          log(Warning)<<this->getName()<<": measurement not reliable."<<endlog();

      _distances_local.resize(datalen);
      for (unsigned int i=0; i<(unsigned int)datalen; i++)
          _distances_local[i] = (double)( (buf[i+1] & 0x1f) <<8  |buf[i]);
      
      _distances.Set(_distances_local);
  }


  void LaserScanner::shutdown()
  {
      _sick_laserscanner->stop();
      marshalling()->writeProperties(_propertyfile);
  }
}//namespace

