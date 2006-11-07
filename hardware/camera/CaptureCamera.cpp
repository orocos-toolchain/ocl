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

#include "CaptureCamera.hpp"
#include <libdc1394/dc1394_control.h>

namespace OCL
{
  
  using namespace RTT;
  using namespace std;
  
  CaptureCamera::CaptureCamera(string name,string propertyfile)
    :TaskContext(name),
     _image("RawImage"),
     _capture_time("CaptureTimestamp"),
     _newImage("updateImage",&CaptureCamera::updateImage,&CaptureCamera::updateImageFinished, this),
     _capture_mode("mode","Capture Mode",MODE_640x480_MONO),
     _capture_shutter("shutter","Capture Shutter",500),
     _capture_gain("gain","Capture Gain",200),
     _capture_convert("convert","Capture Convert",0),
     _capture_fps("fps","Capture Framerate",30),
     _show_time("show_time","True if i have to log capture times",false),
     _show_image("show_image","True if i have to show captured image",false),
     _propertyfile(propertyfile),
     _update(false)
  {
    //    TemplateMethodFactory<CaptureCamera> _my_methodfactory = newMethodFactory( this );
    //    this->methods()->addMethod( method("getImage",&CaptureCamera::getFrame, this),"Capturing new image");
    //    methodFactory.registerObject("this",_my_methodfactory);
  
    this->ports()->addPort(&_image);
    this->ports()->addPort(&_capture_time);
    _empty = cvCreateImage(cvSize(640,480),8,3);
    //Adding Properties
    //=================
    this->properties()->addProperty( &_capture_mode );
    this->properties()->addProperty( &_capture_shutter );
    this->properties()->addProperty( &_capture_gain );
    this->properties()->addProperty( &_capture_convert );
    this->properties()->addProperty( &_capture_fps );
    this->properties()->addProperty( &_show_time );
    this->properties()->addProperty( &_show_image );

    //Adding command
    //==============

    this->commands()->addCommand( &_newImage,"Grab new image and store in DataPort");
        
  }
  
  CaptureCamera::~CaptureCamera()
  {
    cvReleaseImage(&_empty);
  }
  
  bool CaptureCamera::startup()
  {
    
    marshalling()->readProperties("cpf/CaptureCamera.cpf");
          
    //Initialize capturing
    //=====================
    _capture = cvCaptureFromCAM(0);
    if( !_capture )
      {
        Logger::log() << Logger::Error << "(CaptureCamera) Could not initialize capturing..." << Logger::endl;
        return false;
      }
    Logger::log() << Logger::Info << "(CaptureCamera) Capturing started..." << Logger::endl;
  
    //Setting properties for camera
    //=============================
    cvSetCaptureProperty(_capture,CV_CAP_PROP_MODE,_capture_mode.value());
    cvSetCaptureProperty(_capture,FEATURE_SHUTTER,_capture_shutter.value());
    cvSetCaptureProperty(_capture,CV_CAP_PROP_GAIN,_capture_gain.value());    
    cvSetCaptureProperty(_capture,CV_CAP_PROP_CONVERT_RGB,_capture_convert.value());    
    cvSetCaptureProperty(_capture,CV_CAP_PROP_FPS,_capture_fps.value());
  
    //Initialize dataflow
    //===================
    _capture_time.Set(TimeService::Instance()->ticksGet());
    _image.Set(*cvQueryFrame(_capture));
    if(_show_image.value()){
      cvNamedWindow(_image.getName().c_str(),CV_WINDOW_AUTOSIZE);
      cvShowImage(_image.getName().c_str(),&_image.Get());
      cvWaitKey(3);
      cvShowImage(_image.getName().c_str(),&_image.Get());
      cvWaitKey(3);
    }
    
    return true;
  
  }
  
  void CaptureCamera::shutdown()
  {
    if(_show_image.value())
      cvDestroyAllWindows();
    
    Logger::log() << Logger::Info << "(CaptureCamera) stopping Capture..." << Logger::endl;
    cvReleaseCapture( &_capture );
  }
  
  bool CaptureCamera::updateImage()
  {
    return true;
  }
  
  bool CaptureCamera::updateImageFinished()const
  {
      return _update;
  }
  
  void CaptureCamera::update()
  {
    _update = false;
    
    if(_show_time.value()){
      _elapsed = TimeService::Instance()->secondsSince( _timestamp );
      Logger::log()<< Logger::Info << "time since last capture: "<< _elapsed << Logger::endl;
      _timestamp = TimeService::Instance()->ticksGet();
    }
    
    _capture_time.Set(TimeService::Instance()->ticksGet());
    _image.Set(*cvQueryFrame(_capture));
    
    if(_show_image.value())
      cvShowImage(_image.getName().c_str(),&_image.Get());
        
    if(_show_time.value()){
      _elapsed = TimeService::Instance()->secondsSince( _timestamp );
      Logger::log()<< Logger::Info << "time used for capturing: "<< _elapsed << Logger::endl;
      _timestamp = TimeService::Instance()->ticksGet();
    }
    _update = true;
  }
}//namespace


