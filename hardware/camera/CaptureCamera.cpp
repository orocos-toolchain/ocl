// Copyright (C) 2008 Francois Cauwe <francois at lastname dot org>
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
#include <ocl/ComponentLoader.hpp>

// Make this component deployable
ORO_CREATE_COMPONENT(OCL::CaptureCamera)

namespace OCL
{
  
    using namespace RTT;
    using namespace std;
    
    CaptureCamera::CaptureCamera(string name):
        TaskContext(name, PreOperational),
        _image("RawImage"),
        _capture_time("CaptureTimestamp"),
        _newImage("updateImage",&CaptureCamera::updateImage,&CaptureCamera::updateImageFinished, this),
        _setProperties("setProperties",&CaptureCamera::setProperties,&CaptureCamera::setPropertiesFinished, this),
        _camera_index("camera_index","Camera Index",-1),
        _video_driver("video_driver","Video driver: autodetect, v4l or firewire.","firewire"),
        _capture_mode("mode","Capture Mode (firewire), check dc1394 for values",MODE_640x480_MONO),
        _capture_shutter("shutter","Capture Shutter in ms(firewire)",500),
        _capture_gain("gain","Capture Gain",200),
        _capture_convert("convert","Capture Convert to RGB(firewire)",0),
        _capture_fps("fps","Capture Framerate (firewire)",30),
        _capture_width("image_width","Capture frame width (V4L)",640),
        _capture_height("image_height","Capture frame height (V4L)",480),
        _show_time("show_time","True if i have to log capture times (WARNING: only for debugging)",false),
        _show_image("show_image","True if i have to show captured image (WARNING: only for debugging)",false),
        _update(false)
    {
  
        //Adding dataports
        //================
        this->ports()->addPort(&_image);
        this->ports()->addPort(&_capture_time);
    
        //Adding Properties
        //=================
        this->properties()->addProperty( &_camera_index );
        this->properties()->addProperty( &_video_driver );
        this->properties()->addProperty( &_capture_mode );
        this->properties()->addProperty( &_capture_shutter );
        this->properties()->addProperty( &_capture_gain );
        this->properties()->addProperty( &_capture_convert );
        this->properties()->addProperty( &_capture_fps );
        this->properties()->addProperty( &_capture_width );
        this->properties()->addProperty( &_capture_height );
        this->properties()->addProperty( &_show_time );
        this->properties()->addProperty( &_show_image );
        
        //Adding command
        //==============
        this->commands()->addCommand( &_newImage,"Grab new image and store in DataPort");
        this->commands()->addCommand( &_setProperties,"Commit new capture properties to camera");
        
    }
    
    CaptureCamera::~CaptureCamera()
    {
        //nothing to do
    }
    
    
    bool CaptureCamera::configureHook()
    {
        Logger::In in(this->getName().data());
        
        // Read config file
        //=================
        if ( this->marshalling()->readProperties( this->getName() + ".cpf" ) == false)
            return false;
        
        //Initialize capturing
        //====================
        if (_video_driver.value()=="firewire"){
            _capture = cvCaptureFromCAM(_camera_index.value()+CV_CAP_IEEE1394);
        }else if(_video_driver.value()=="v4l"){
            _capture = cvCaptureFromCAM(_camera_index.value()+CV_CAP_V4L);
        }else if(_video_driver.value()=="autodetect"){
            _capture = cvCaptureFromCAM(_camera_index.value());
        }else{
            log(Error) << "Unkown video driver! Please choose firewire, v4l or autodetect." << endlog();
            return false;
        }
        
        if( !_capture ){
            log(Error) << "Could not initialize capturing..." << endlog();
            return false;
        }
        
        // Set properties
        //===============
        this->setProperties();
        
        // Grab one frame
        //===============
        _frame=cvQueryFrame(_capture);
        _image.Set(*_frame);
        if(_show_image.value()){
            cvNamedWindow(this->getName().c_str(),CV_WINDOW_AUTOSIZE);
            cvWaitKey(3);//wait 3 ms for main loop of graphical server
            cvShowImage(this->getName().c_str(),_frame);
            cvWaitKey(3);//wait 3 ms for main loop of graphical server
        }
        
        log(Info) << "Capturing initialized..." << endlog();
        return true;
    }
    
    
    
    bool CaptureCamera::startHook()
    {          
        //Start capturing
        //===============
        _capture_time.Set(TimeService::Instance()->ticksGet());
        
        return true;
    }
    
    void CaptureCamera::cleanupHook()
    {
        Logger::In in(this->getName().data());
        
        // Close window
        if(_show_image.value()){
            cvDestroyWindow(this->getName().c_str());
            cvWaitKey(3);//wait 3 ms for main loop of graphical server
        }
        
        // Release capture device
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
    
    void CaptureCamera::updateHook()
    {
        Logger::In in(this->getName().data());
        
        _update = false;
        
        // Show time since last capture
        if(_show_time.value()){
            _elapsed = TimeService::Instance()->secondsSince( _timestamp );
            log(Info) << "time since last capture: "<< _elapsed << endlog();
            _timestamp = TimeService::Instance()->ticksGet();
        }
        
        // Capture new frame
        _capture_time.Set(TimeService::Instance()->ticksGet());
        
        if(!cvGrabFrame(_capture)){
            log(Error) << "Failed to grab a new frame." << endlog();
            this->error();
        }
        _frame = cvRetrieveFrame(_capture);
        
        _image.Set(*_frame);
        
        // Show image 
        if(_show_image.value()){
            cvShowImage(this->getName().c_str(),_frame);
            cvWaitKey(3);//wait 3 ms for main loop of graphical server
        }
        
        // Show time used for capturing
        if(_show_time.value()){
            _elapsed = TimeService::Instance()->secondsSince( _timestamp );
            log(Info) << "time used for capturing: "<< _elapsed << endlog();
            _timestamp = TimeService::Instance()->ticksGet();
        }
        
        _update = true;
    }
    
    
    bool CaptureCamera::setProperties()
    {
        Logger::In in(this->getName().data());
        
        _update_properties=false;
        
        //Setting properties for camera
        //=============================
        
        // Global settings
        if(cvSetCaptureProperty(_capture,CV_CAP_PROP_GAIN,_capture_gain.value())<1)
            log(Warning) << "Could not set the capturing gain property!" << endlog();
    
        // Settings only for firewire
        if(_video_driver.value()=="firewire"){
            if(cvSetCaptureProperty(_capture,CV_CAP_PROP_MODE,_capture_mode.value())<1)
                log(Warning) << "Could not set the capturing mode property!" << endlog();
            if(cvSetCaptureProperty(_capture,FEATURE_SHUTTER,_capture_shutter.value())<1)
                log(Warning) << "Could not set the shutter speed property!" << endlog();
            if(cvSetCaptureProperty(_capture,CV_CAP_PROP_CONVERT_RGB,_capture_convert.value())<1)
                log(Warning) << "Could not set the RGB conversion property!" << endlog();
            if(cvSetCaptureProperty(_capture,CV_CAP_PROP_FPS,_capture_fps.value())<1)
                log(Warning) << "Could not set the fps property!" << endlog();
        }
        
        // Settings only for V4L
        if(_video_driver.value()=="v4l"){
            // First set width then height. The first command always returns a error!
            cvSetCaptureProperty(_capture,CV_CAP_PROP_FRAME_WIDTH,_capture_width.value());
            if(cvSetCaptureProperty(_capture,CV_CAP_PROP_FRAME_HEIGHT,_capture_height.value())<1)
                log(Warning) <<"Could not set the frame width and height property!" << endlog();
        }
        
        _update_properties = true;
        return true;
    }
    
    bool CaptureCamera::setPropertiesFinished()const
    {
        return _update_properties;
    }
}//namespace


