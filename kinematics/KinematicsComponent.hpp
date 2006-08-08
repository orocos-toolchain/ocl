/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006
  KinematicProcess.hpp 

                        KinematicProcess.hpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be
 
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
 
 
#ifndef _KINEMATIC_COMPONENT_HPP_
#define _KINEMATIC_COMPONENT_HPP_


#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <kdl/kinfam/kinematicfamily.hpp>
#include <rtt/PropertyComposition.hpp>
#include <rtt/RTT.hpp>

namespace Orocos
{

    /**
     * A Component which offers a kinematics interface to
     * other components.
     * Use the properties of this component to configure where the
     * data can be found and under which names, as well as the kinematic
     * algorithm to use.
     *
	 *   In the likely case that your robot type is not supported,
	 *   you can create your own type using KDL. For information look
	 *   at www.orocos.org/kdl
     */
    class KinematicsComponent
        : public RTT::GenericTaskContext
    {
        KDL::KinematicFamily*      kf;
        KDL::Jnt2CartPos*          jnt2cartpos;
        KDL::CartPos2Jnt*          cartpos2jnt;
        KDL::Jnt2Jac*              jnt2jac;
        KDL::Jnt2CartVel*          jnt2CartVel;
        KDL::CartVel2Jnt*          CartVel2Jnt;
        
        RTT::Property<std::string> kinarch;
        RTT::ReadDataPort<KDL::JointVector>  qPort;
        RTT::ReadDataPort<KDL::JointVector> qdotPort;

        KDL::JointVector tmppos;
    public:
        /**
         * A Component which reads joint positions and velocities and converts
         * them into an end effector frame and twist using a kinematic algorithm.
         * @param name The name of this component in the control kernel.
         */
        KinematicsComponent(const std::string& name, KDL::KinematicFamily* _kf);

        /**
         * Returns the name of the current architecture.
         */
        const std::string& getArchitecture() const;

        /**
         * Returns an object which can be used to track and perform
         * kinematic algorithms for other Kernel components.
         * Use also this method if you want to manually load a new
         * kinematics algorithm into the KinematicsComponent.
         */
        KDL::KinematicFamily* getKinematics();

        /**
         * Set the Kinematic Algorithm to use and optional axis velocity conversions and axis position offsets.
         */
        void setKinematics( KDL::KinematicFamily *_kf);

        bool startup();
        
        void update();

        
        
    };

}

#endif
