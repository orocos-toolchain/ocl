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

#ifndef __CAPTURECAMERA_HPP__
#define __CAPTURECAMERA_HPP__

#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>

#include <rtt/TimeService.hpp>
#include <rtt/Time.hpp>

#include <opencv/cv.h>
#include <opencv/highgui.h>
namespace Orocos
{
  
  class CaptureCamera : public RTT::GenericTaskContext 
  {
    
  public:
    CaptureCamera(std::string name,std::string propertyfile="cpf/CaptureCamera");
    virtual ~CaptureCamera();
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
        
  private:

    virtual bool updateImage();
    virtual bool updateImageFinished()const;
    

    std::string                          _propertyfile;
    
    CvCapture                            *_capture;
      
    RTT::WriteDataPort<IplImage> _image;
    RTT::WriteDataPort<RTT::TimeService::ticks> _capture_time;
        
    RTT::TimeService::ticks             _timestamp;
    RTT::Seconds                        _elapsed;
  
    RTT::Property<int>                  _capture_mode, _capture_shutter,_capture_gain,_capture_convert;
    RTT::Property<double>               _capture_fps;
    RTT::Property<bool>                 _show_time;
    RTT::Property<bool>                 _show_image;
    
    IplImage* _empty;
    bool _update;
    
  };
}//namespace

#endif // CAPTURECAMERA_HPP

