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
#define M_TO_MM             1000.0

namespace Orocos
{
  using namespace RTT;
  using namespace std;
  using namespace KDL;
  using namespace MatrixWrapper;
  
  
  Demotool::Demotool(string name, string propertyfile):
    GenericTaskContext(name),
    _pos_leds_demotool("pos_leds_demotool","XYZ positions of all LED markers, relative to demtool frame"),
    _mass_demotool("mass_demotool","mass of objects attached to force censor of demotool"),
    _center_gravity_demotool("center_gravity_demotool","center of gravity of mass attached to demotool"),
    _Frame_demotool_obj("demotool_obj","frame from demotool to object"),
    _Frame_demotool_fs("demotool_fs","frame from demotool to force sensor"),
    _Frame_world_camera("world_camera","frame from world to camera"),
    _Wrench_fs_fs_port("WrenchData"),
    _Vector_led_camera_port("LedPositions"),
    _Wrench_world_world_port("wrench_world_world"),
    _Wrench_obj_obj_port("wrench_obj_obj"),
    _Twist_obj_world_port("twit_obj_world"),
    _Frame_world_obj_port("frame_world_obj"),
    _num_visible_leds_port("num_visible_leds"),
    _propertyfile(propertyfile)
  {
    Logger::log()<<Logger::Debug<<this->getName()<<": adding Properties"<<Logger::endl;
    properties()->addProperty(&_pos_leds_demotool);
    properties()->addProperty(&_mass_demotool);
    properties()->addProperty(&_center_gravity_demotool);
    properties()->addProperty(&_Frame_demotool_obj);
    properties()->addProperty(&_Frame_demotool_fs);
    properties()->addProperty(&_Frame_world_camera);

    Logger::log()<<Logger::Debug<<this->getName()<<": adding Ports"<<Logger::endl;
    ports()->addPort(&_Wrench_fs_fs_port);
    ports()->addPort(&_Vector_led_camera_port);
    ports()->addPort(&_Wrench_world_world_port);
    ports()->addPort(&_Wrench_obj_obj_port);
    ports()->addPort(&_Twist_obj_world_port);
    ports()->addPort(&_Frame_world_obj_port);
    ports()->addPort(&_num_visible_leds_port);
    
    Logger::log()<<Logger::Debug<<this->getName()<<": adding Events"<<Logger::endl;
    //events()->addEvent("distanceOutOfRange",&_distanceOutOfRange);

    Logger::log()<<Logger::Debug<<this->getName()<<": adding Methods"<<Logger::endl;
    methods()->addMethod(method("resetPosition", &Demotool::resetPosition, "set world frame to current object frame"));

    if (!readProperties(_propertyfile))
      log(Error)<<"Reading properties failed."<<endlog();
    
    // foce component of gravity
    _Wrench_gravity_world_world.force  = KDL::Vector(0, 0, (-1 * _mass_demotool * GRAVITY_CONSTANT));

    // number of leds as found in property file
    _num_leds = (int) (_pos_leds_demotool.value().size()/3);
    assert( _pos_leds_demotool.size() = 3*_num_leds);
    _visible_leds.resize(_num_leds);
    _Vector_led_demotool.resize(_num_leds);

    // copy vector<double> to vector<Vector>
    Vector temp;
    for (unsigned int i=0; i<_num_leds; i++){
      for (unsigned int j=0; j<3; j++)
	temp(j) = _pos_leds_demotool.value()[(3*i)+j];
      _Vector_led_demotool[i] = temp;
    }

    Logger::log()<<Logger::Debug<<this->getName()<<": constructed."<<Logger::endl;
  }
  
  Demotool::~Demotool()
  {
  }
  

  bool Demotool::startup()    
  {
    // set _Twist_obj_world to zero
    SetToZero(_Twist_obj_world);

    // re-initialize twist calculation
    _is_initialized = false;

    return true;
  }

    
  void Demotool::update()
  {
    // get force sensor measurement
    // ----------------------------
    _Wrench_fs_fs = _Wrench_fs_fs_port.Get();
    
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
      Matrix residu = (trans_camera_demotool * matrix_leds_demotool) - matrix_leds_camera;
      for (unsigned int i=0; i<_num_visible_leds; i++)
	for (unsigned int j=0; j<3; j++)
	  vec_temp(j) = residu(j+1, i+1);
	error += vec_temp.Norm() /_num_visible_leds;
      if (error > 0.001){
	Logger::log() << Logger::Warning << "SensorDemotool: Error while solving LED position equation = " 
		      << error << " > 0.001" << Logger::endl;
      }
    }
    

    // twist of obj expressed in world, refpoint obj
    // ---------------------------------------------
    _period = TimeService::Instance()->secondsSince(_time_begin);
    _Frame_world_obj = _Frame_world_demotool * _Frame_demotool_obj.value();
    if (_is_initialized)
      _Twist_obj_world = diff(_Frame_world_obj_old, _Frame_world_obj, _period);
    else
      _is_initialized = true;
    _time_begin = TimeService::Instance()->getTicks();
    _Frame_world_obj_old = _Frame_world_obj;

    
    // mass compensation for force sensor
    // ----------------------------------
    _Frame_world_fs = _Frame_world_demotool * _Frame_demotool_fs.value();
    _Wrench_gravity_world_world.torque = (_Frame_world_demotool * _center_gravity_demotool.value())
                                         * _Wrench_gravity_world_world.force;
    _Wrench_world_world = _Frame_world_fs * _Wrench_fs_fs;


    // copy data to ports
    // ------------------
    _Frame_world_obj_port.Set(_Frame_world_obj);
    _Wrench_world_world_port.Set(_Wrench_world_world);
    _Wrench_obj_obj_port.Set(_Frame_world_obj.Inverse() * _Wrench_world_world);
    _Twist_obj_world_port.Set(_Twist_obj_world);
    _num_visible_leds_port.Set(_num_visible_leds);
  }
  
  void Demotool::shutdown()
  {
    writeProperties(_propertyfile);
  }


  bool Demotool::resetPosition()
  {
    // set frame world_camera to obj_camera ==> world frame in current object frame
    _Frame_world_camera.value() = _Frame_demotool_obj.value().Inverse() * _Frame_camera_demotool.Inverse();

    return true;
  }


}//namespace

