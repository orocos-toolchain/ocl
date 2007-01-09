// Copyright (C) 2006 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
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

#include "Demotool.hpp"
#include <matrix_wrapper.h>
#include <kdl/frames_io.hpp>

#define GRAVITY_CONSTANT    9.81
#define MM_TO_M             0.001

namespace OCL
{
  using namespace RTT;
  using namespace std;
  using namespace KDL;
  using namespace MatrixWrapper;
  
  
  Demotool::Demotool(string name, string propertyfile):
    TaskContext(name),
    _pos_leds_demotool("pos_leds_demotool","XYZ positions of all LED markers, relative to demtool frame"),
    _mass_demotool("mass_demotool","mass of objects attached to force censor of demotool"),
    _center_gravity_demotool("center_gravity_demotool","center of gravity of mass attached to demotool"),
    _gravity_dir_world("gravity_dir_world",""),
    _Frame_demotool_manip("demotool_manip","frame from demotool to manip"),
    _Frame_demotool_fs("demotool_fs","frame from demotool to force sensor"),
    _Frame_world_camera("world_camera","frame from world to camera"),
    _Wrench_fs_fs_port("WrenchData"),
    _Vector_led_camera_port("LedPositions"),
    _Wrench_world_world_port("wrench_world_world"),
    _Wrench_manip_manip_port("wrench_manip_manip"),
    _Twist_world_world_port("twit_world_world"),
    _Twist_world_manip_port("twist_world_manip"),
    _Frame_world_manip_port("frame_world_manip"),
    _num_visible_leds_port("num_visible_leds"),
    _calibrate_world_to_manip("calibrateWorldToManip", &Demotool::calibrateWorldToManip, this),
    _calibrate_wrench_sensor("calibrateWrenchSensor", &Demotool::calibrateWrenchSensor, this),
    _propertyfile(propertyfile)
  {
    log(Debug) <<this->getName()<<": adding Properties"<<endlog();
    properties()->addProperty(&_pos_leds_demotool);
    properties()->addProperty(&_mass_demotool);
    properties()->addProperty(&_center_gravity_demotool);
    properties()->addProperty(&_gravity_dir_world);
    properties()->addProperty(&_Frame_demotool_manip);
    properties()->addProperty(&_Frame_demotool_fs);
    properties()->addProperty(&_Frame_world_camera);

    log(Debug) <<this->getName()<<": adding Ports"<<endlog();
    ports()->addPort(&_Wrench_fs_fs_port);
    ports()->addPort(&_Vector_led_camera_port);
    ports()->addPort(&_Wrench_world_world_port);
    ports()->addPort(&_Wrench_manip_manip_port);
    ports()->addPort(&_Twist_world_world_port);
    ports()->addPort(&_Twist_world_manip_port);
    ports()->addPort(&_Frame_world_manip_port);
    ports()->addPort(&_num_visible_leds_port);
    
    log(Debug) <<this->getName()<<": adding Methods"<<endlog();
    methods()->addMethod(_calibrate_world_to_manip, "set world frame to current manip frame");
    methods()->addMethod(_calibrate_wrench_sensor, "set wrench sensor offset to measure zero force");

    if (!marshalling()->readProperties(_propertyfile)) {
      log(Error) << "Failed to read the property file, continueing with default values." << endlog();
    }  
    
    // foce component of gravity
    _Wrench_gravity_world_world.force  = _gravity_dir_world.value() * (_mass_demotool.value() * GRAVITY_CONSTANT);

    // number of leds as found in property file
    _num_leds = (int) (_pos_leds_demotool.value().size()/3);
    assert( _pos_leds_demotool.value().size() == 3*_num_leds);
    _visible_leds.resize(_num_leds);
    _Vector_led_demotool.resize(_num_leds);

    // copy vector<double> to vector<Vector>
    Vector temp;
    for (unsigned int i=0; i<_num_leds; i++){
      for (unsigned int j=0; j<3; j++)
	temp(j) = _pos_leds_demotool.value()[(3*i)+j];
      _Vector_led_demotool[i] = temp;
    }

    log(Debug) <<this->getName()<<": constructed."<<endlog();
  }
  
  Demotool::~Demotool()
  {
  }
  

  bool Demotool::startup()    
  {
    // get command from wrench component, to add offset
    TaskContext* wrench_sensor = getPeer("Wrenchsensor");
    if (!wrench_sensor) log(Error) <<this->getName()<<": peer Wrenchsensor not found."<<endlog();
 
    _add_offset = wrench_sensor->commands()->getCommand<bool(Wrench)>("addOffset");
    if (!_add_offset.ready()) log(Error) <<this->getName()<<": command addOffset not found."<<endlog();

    // set _Twist_world_manip to zero
    SetToZero(_Twist_world_manip);

    // re-initialize twist calculation
    _is_initialized = false;

    return true;
  }

    
  void Demotool::update()
  {
    // get krypton LED positions
    // -------------------------
    _num_visible_leds = 0;
    _Vector_led_camera = _Vector_led_camera_port.Get();
    assert(_Vector_led_camera.size() == _Vector_led_demotool.size());
    for (unsigned int i=0; i<_num_leds; i++){
      bool visible = true;
      for (unsigned int j=0; j<3; j++){
	double temp_double = _Vector_led_camera[i](j);
	if (temp_double == -99999 || temp_double == 0) visible = false;
      }
      _visible_leds[i] = visible;
      if (visible) _num_visible_leds++;
    }

    // calculate frame_camera-demotool
    // -------------------------------
    if (_num_visible_leds >= 4){
      unsigned int pos = 0;
      Matrix matrix_leds_camera(3, _num_visible_leds);
      Matrix matrix_leds_demotool(4, _num_visible_leds);
      for (unsigned int i=0; i<_num_leds; i++){
	if (_visible_leds[i]){
	  for (unsigned int j=0; j<3; j++){
	    matrix_leds_camera  (j+1, pos+1) = _Vector_led_camera  [i](j) * MM_TO_M;
	    matrix_leds_demotool(j+1, pos+1) = _Vector_led_demotool[i](j);
	  }
	  matrix_leds_demotool(4, pos+1) = 1;
	  pos++;
	}
      }
      Matrix trans_camera_demotool = matrix_leds_camera * (matrix_leds_demotool.pseudoinverse());
      Rotation rot_temp; double roll, pitch, yaw;
      for (unsigned int i=0; i<3; i++){
	for (unsigned int j=0; j<3; j++)
	  rot_temp(i,j) = trans_camera_demotool(i+1,j+1);
	_Frame_camera_demotool.p(i) = trans_camera_demotool(i+1,4);
      }
      rot_temp.GetRPY(roll, pitch, yaw);
      _Frame_camera_demotool.M = Rotation::RPY(roll, pitch, yaw);
      _Frame_world_demotool = _Frame_world_camera.value() * _Frame_camera_demotool;


      // check error on frame_camera-demotool
      double error = 0;  Vector vec_temp;
      for (unsigned int i=0; i<3; i++)
	for (unsigned int j=0; j<3; j++)
	  trans_camera_demotool(i+1,j+1) = _Frame_camera_demotool.M(i,j);
      Matrix residu = (trans_camera_demotool * matrix_leds_demotool) - matrix_leds_camera;
      for (unsigned int i=0; i<_num_visible_leds; i++)
	for (unsigned int j=0; j<3; j++)
	  vec_temp(j) = residu(j+1, i+1);
	error += vec_temp.Norm() /_num_visible_leds;
      if (error > 0.001){
	log(Warning) << "SensorDemotool: Error while solving LED position equation = " 
		      << error << " > 0.001" << endlog();
      }
    }
    

    // twist of manip expressed in world, refpoint manip
    // ---------------------------------------------
    _period = TimeService::Instance()->secondsSince(_time_begin);
    _Frame_world_manip = _Frame_world_demotool * _Frame_demotool_manip.value();
    if (_is_initialized)
      _Twist_world_manip = diff(_Frame_world_manip_old, _Frame_world_manip, _period);
    else
      _is_initialized = true;
    _time_begin = TimeService::Instance()->getTicks();
    _Frame_world_manip_old = _Frame_world_manip;

    
    // get force sensor measurement
    // ----------------------------
    _Wrench_fs_fs = _Wrench_fs_fs_port.Get();
    // mass compensation for force sensor
    _Frame_world_fs = _Frame_world_demotool * _Frame_demotool_fs.value();
    _Wrench_gravity_world_world.torque = (_Frame_world_demotool * _center_gravity_demotool.value())
                                         * _Wrench_gravity_world_world.force;
    _Wrench_world_world = (_Frame_world_fs * _Wrench_fs_fs) - _Wrench_gravity_world_world;


    // copy data to ports
    // ------------------
    _Frame_world_manip_port.Set(_Frame_world_manip);
    _Wrench_world_world_port.Set(_Wrench_world_world);
    _Wrench_manip_manip_port.Set(_Frame_world_manip.Inverse() * _Wrench_world_world);
    _Twist_world_world_port.Set(_Twist_world_manip.RefPoint(-(_Frame_world_manip.p)));
    _Twist_world_manip_port.Set(_Twist_world_manip);
    _num_visible_leds_port.Set(_num_visible_leds);
  }
  
  void Demotool::shutdown()
  {
    marshalling()->writeProperties(_propertyfile);
  }


  void Demotool::calibrateWorldToManip()
  {
    // set frame world_camera to manip_camera ==> world frame in current manip frame
    _Frame_world_camera.value() = _Frame_demotool_manip.value().Inverse() * _Frame_camera_demotool.Inverse();
  }

  void Demotool::calibrateWrenchSensor()
  {
    log(Debug) <<this->getName()<<": calibrate with " << _Frame_world_fs.Inverse() * _Wrench_world_world 
		 <<endlog();

    _add_offset( _Frame_world_fs.Inverse() * _Wrench_world_world );
  }



}//namespace

