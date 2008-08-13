/***************************************************************************

                        Vision.hpp -  description
                        -------------------------

    begin                : Jan 2008
    copyright            : (C) 2008 Francois Cauwe
    email                : francois@cauwe.org

    ***************************************************************************
    *   This library is free software; you can redistribute it and/or         *
    *   modify it under the terms of the GNU Lesser General Public            *
    *   License as published by the Free Software Foundation; either          *
    *   version 2.1 of the License, or (at your option) any later version.    *
    *                                                                         *
    *   This library is distributed in the hope that it will be useful,       *
    *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
    *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
    *   Lesser General Public License for more details.                       *
    *                                                                         *
    *   You should have received a copy of the GNU Lesser General Public      *
    *   License along with this library; if not, write to the Free Software   *
    *   Foundation, Inc., 59 Temple Place,                                    *
    *   Suite 330, Boston, MA  02111-1307  USA                                *
    *                                                                         *
    ***************************************************************************/

#ifndef __VISIONTOOLKIT__
#define __VISIONTOOLKIT__

// Toolkit
#include <rtt/TemplateTypeInfo.hpp>

// OpenCV includes for displaying
#include <opencv/cv.h>
#include <opencv/highgui.h>


namespace OCL
{
    // Define the image type
    struct IplImageTypeInfo
        : public RTT::TemplateTypeInfo<IplImage>
    {
        IplImageTypeInfo()
            : RTT::TemplateTypeInfo<IplImage>("IplImage")
        {}
    };

}

#endif
