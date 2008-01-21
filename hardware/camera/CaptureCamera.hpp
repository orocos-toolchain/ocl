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

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Command.hpp>

#include <rtt/TimeService.hpp>
#include <rtt/Time.hpp>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <ocl/OCL.hpp>

namespace OCL
{
    
    /**
     * This class implements a TaskContext that grabs
     * images from a firewire-camera using OpenCV. The component
     * can be executed a periodic as well as by a non-periodic
     * activity.
     *
     */
    
    class CaptureCamera : public RTT::TaskContext 
    {
    
    public:
        /** 
         * Creates an unconfigured CaptureCamera component. 
         * 
         * @param name Name of the component
         * 
         * @return
         */
        CaptureCamera(std::string name);
        virtual ~CaptureCamera();
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void cleanupHook();
        
    protected:
        /// Dataport which contains grabbed image
        RTT::WriteDataPort<IplImage>        _image;
        /// Dataport which contains grabbing timestamp
        RTT::WriteDataPort<RTT::TimeService::ticks> _capture_time;
        /// Command to grab image
        RTT::Command<bool(void)>            _newImage;
        /// Command to update the camera properties
        RTT::Command<bool(void)>            _setProperties;

        /// camera id
        RTT::Property<int>                  _camera_index;
        /// select video driver: autodetect, v4l or firewire
        RTT::Property<std::string>          _video_driver;
        /// capturing mode, check dc1394 for values (firewire)
        RTT::Property<int>                  _capture_mode;
        /// shutter time in micro seconds (firewire)
        RTT::Property<int>                  _capture_shutter;
        /// capturing gain
        RTT::Property<int>                  _capture_gain;
        /// set true if opencv should convert the grabbed image to RGB (firewire)
        RTT::Property<int>                  _capture_convert;
        /// framerate of camera (firewire)
        RTT::Property<double>               _capture_fps;
        /// capture frame width
        RTT::Property<int>                  _capture_width;
        /// capture frame height
        RTT::Property<int>                  _capture_height;
        /// boolean whether to print capturing time information
        RTT::Property<bool>                 _show_time;
        /// boolean whether to show the captured image in a window
        RTT::Property<bool>                 _show_image;

    private:
        virtual bool updateImage();
        virtual bool updateImageFinished()const;
        virtual bool setProperties();
        virtual bool setPropertiesFinished()const;
        
        CvCapture                            *_capture;
        RTT::TimeService::ticks             _timestamp;
        RTT::Seconds                        _elapsed;
        
        IplImage*                           _frame;
        bool _update;
        bool                                _update_properties;
    };
}//namespace

#endif // CAPTURECAMERA_HPP

