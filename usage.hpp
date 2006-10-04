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

/** 
* \mainpage Orocos-Components
* 
* \section intro Introduction
*
* This project consists of a series of independent Orocos
* components which can be used to build a control application.
* It uses the Orocos RealTime Toolkit and KDL, a kinematic and dynamic
* library which is available through svn at
* http://svn.mech.kuleuven.be/repos/orocos/kdl/trunk/kdl
*
* Structure:
* 
* \subsection reporting Reporting: [ready]
* 	Components for real-time data capturing. Currently console
* 	and file reporting is supported.
* 
* \subsection taskbrowser Taskbrowser: [ready]
* 	A Component for online interaction with any other component.
* 	Your main tool during component development, testing or
* 	experimenting.
* 
* \subsection viewer Viewer: [beta stage]
*   A Component which sends naxisposition values to the
* kdlviewer. KDLViewer is an application of KDL and can be found on
* http://svn.mech.kuleuven.be/repos/orocos/kdl/trunk/apps
* 
* \subsection hardware Hardware: [beta stage] 
*   Components for various devices (like sensors)
* 	and hardware platforms, the simulation devices can be used by
* 	everyone,
*   <ul>
*     <li>kuka: Implementation for use with Kuka160 and Kuka361,
*         simulation can be used for everyone, realtime execution only in our
*         lab (KULeuven/PMA/robotics) since the interface to the different
*         DAQ-cards is very specific.
*     <li>wrench: Implementation for use with JR3-sensor, can be used
*         by everyone using the pci-interface to a JR3-sensor. The
*         specific lxrt-drivers can be found in the driver subdirectory.
* 	  <li>camera: Implementation for use with firewire camera's and
*        OpenCV.
*     <li>krypton: Implementation to get Krypton (see www.metris.com)
*         measurements in realtime. The specific lxrt-drivers can be found in
*         the driver subdirectory.
*     <li>demotool: Implementation to combine Krypton and WrenchSensor
*        measurement for the human demonstration tool in our lab (KULeuvenn/PMA/Robotics)
* 	
* 
* \subsection motion Motion_control: [beta stage]
* 	Components for path interpolation, cartesian and joint space
* 	control. These components need KDL.
* 
* \subsection deployment Deployment: [alfa stage]
* 	Component for connecting and configuring an application.
* 
* \subsection kinematics kinematics: [deprecated]
* 	Component for forward and reverse kinematics. Needs to
* 	be adapted to the KDL. Not used in any testcase at the moment.
*  
*/
