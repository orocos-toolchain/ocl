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
  
  LaserScanner::LaserScanner(string name, string propertyfile):
    TaskContext(name),
    _simulation_values("sim_values","Value used for simulation",vector<double>(nr_chan,0)),
    _distances("LaserDistance",vector<double>(nr_chan,0)),
    _distanceOutOfRange("distanceOutOfRange"),
    _propertyfile(propertyfile)
  {
    log(Debug) <<this->getName()<<": adding Properties"<<endlog();
    properties()->addProperty(&_simulation_values);
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
    if (_range_mode.value() == 100 ) _range_mode_char = SickLMS200::RANGE_100;
    else if (_range_mode.value() == 180 ) _range_mode_char = SickLMS200::RANGE_180;
    else {}
    
    

    case 'r':
      if(strncmp(optarg,"180",3)==0) range_mode=SickLMS200::RANGE_180;
      else range_mode=SickLMS200::RANGE_100;
      break;
    case 's':
      if(strncmp(optarg,"0.25",4)==0) res_mode=SickLMS200::RES_0_25_DEG;
      else if(strncmp(optarg,"0.5",3)==0) res_mode=SickLMS200::RES_0_5_DEG;
      else res_mode=SickLMS200::RES_1_DEG;
      break;
    case 'u':
      if(strncmp(optarg,"cm",2)==0) unit_mode=SickLMS200::CMMODE;
      else unit_mode=SickLMS200::MMMODE;
      break;


    _sick_laserscanner = new SickLMS200(_port.value(), _range_mode.value(), _res_mode.value(), _unit_mode.value());

    log(Debug) <<this->getName()<<": constructed."<<endlog();
  }

  
  LaserScanner::~LaserScanner()
  {
      delete _sick_laserscanner;
  }
  

  bool LaserScanner::startup()    
  {
      _sick_laserscanner->start();
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
      showdata(datalen,buf);

      _distances_local = _simulation_values.value();
      _distances.Set(_distances_local);
  }


  void LaserScanner::shutdown()
  {
      marshalling()->writeProperties(_propertyfile);
  }
}//namespace

