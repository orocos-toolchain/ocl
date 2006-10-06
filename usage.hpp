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
* Examples for the usage of all components can be found in the
* respective tests subdirectory 
*
* \section structure Structure:
* 
* \subsection reporting Reporting: [ready]
* 	Components for real-time data capturing. Currently console
* 	and file reporting is supported.
*   <ul>
*   <li>Orocos::ReportingComponent
*   <li>Orocos::ConsoleReporting
*   <li>Orocos::FileReporting
*   </ul>
* \subsection taskbrowser Taskbrowser: [ready]
* 	A Component for online interaction with any other component.
* 	Your main tool during component development, testing or
* 	experimenting.
*   <ul>
*   <li>Orocos::TaskBrowser
*   </ul>
*
* \subsection viewer Viewer: [beta stage]
*   A Component which sends naxisposition values to the
*   kdlviewer. KDLViewer is an application of KDL and can be found on
*   http://svn.mech.kuleuven.be/repos/orocos/kdl/trunk/apps
*   <ul>
*   <li>Orocos::NAxesPositionViewer
*   </ul>
*
* \subsection hardware Hardware: [ready] 
*   Components for various devices (like sensors)
* 	and hardware platforms, the simulation devices can be used by
* 	everyone,
*   <ul>
*     <li>kuka: Implementation for use with Kuka160 and Kuka361,
*         simulation can be used for everyone, realtime execution only in our
*         lab (KULeuven/PMA/robotics) since the interface to the different
*         DAQ-cards is very specific. A software EmergencyStop is
*         implemented to safely stop the robots.
*         <ul>
*         <li> Orocos::Kuka160nAxesVelocityController
*         <li> Orocos::Kuka361nAxesVelocityController
*         <li> Orocos::EmergencyStop
*         </ul>
*
*     <li>wrench: Implementation for use with JR3-sensor, can be used
*         by everyone using the pci-interface to a JR3-sensor. The
*         specific lxrt-drivers can be found in the driver
*         subdirectory.
*         <ul>
*         <li> Orocos::WrenchSensor
*         </ul>
*
* 	  <li>camera: Implementation for use with firewire camera's and
*        OpenCV. Can be used by everyone.
*        <ul>
*        <li> Orocos::CaptureCamera
*        </ul>
*
*     <li>krypton: Implementation to get Krypton (see www.metris.com)
*         measurements in realtime. The specific lxrt-drivers can be found in
*         the driver subdirectory.
*         <ul>
*         <li>Orocos::KryptonK600Sensor
*         </ul>
*
*     <li>demotool: Implementation to combine Krypton and WrenchSensor
*        measurement for the human demonstration tool in our lab (KULeuven/PMA/Robotics)
* 	      <ul>
*         <li>Orocos::Demotool
*         </ul> 
*
*     <li>laserdistance: Implementation to read and convert the LaserSensor
*     measurements. Can only be used through the NI-6024 pci card in
*     our lab(KULeuven/PMA/Robotics). But can be an example for everyone.
*         <ul>
*         <li>Orocos::LaserSensor
*         </ul>
*     </ul>
*
* \subsection motion Motion_control: [ready]
* 	Components for path interpolation and control in cartesian and joint space.
* 	These components need KDL for the path interpolation and for the
*   cartesian operations.
*   <ul>
*   <li> naxes: These components include different controllers and
*        generators(interpolators) and an effector(output) and sensor(input)
*        which sends velocity setpoints to and reads position measurements
*        from dataports shared with one of the
*        hardware-components. It can be used for any number of axes.
*        <ul>
*           <li>Orocos::nAxesSensor
*           <li>Orocos::nAxesGeneratorPos
*           <li>Orocos::nAxesGeneratorVel
*           <li>Orocos::nAxesControllerPos
*           <li>Orocos::nAxesControllerPosVel
*           <li>Orocos::nAxesControllerVel
*           <li>Orocos::nAxesEffectorVel
*           <li>Orocos::ReferenceSensor
*        </ul>
*   <li> cartesian: These components include different controllers and
*        generators(interpolators) that control cartesian setpoints
*        in 6D (3 translations and 3 rotations). It uses a
*        KDL::KinematicFamily (see KDL) to convert the axes positions to
*        cartesian position and the cartesian velocity to axes
*        velocities. It gets and sends its position and velocities
*        to dataports shared with one of the hardware-components. It
*        can be used for any number of axes.  
*        <ul>
*           <li>Orocos::CartesianSensor
*           <li>Orocos::CartesianGeneratorPos
*           <li>Orocos::CartesianControllerPos
*           <li>Orocos::CartesianControllerPosVel
*           <li>Orocos::CartesianControllerVel
*           <li>Orocos::CartesianEffectorVel
*        </ul>
*   </ul>  
*
* \subsection deployment Deployment: [alfa stage]                  
* 	Component for connecting and configuring an application.      
*   <ul>
*   <li>Orocos::DeploymentComponent
*   </ul>
*                                                               
* \subsection kinematics kinematics: [deprecated]                
* 	Component for forward and reverse kinematics. Needs to        
* 	be adapted to the KDL. Not used in any testcase at the moment.
*
*/
