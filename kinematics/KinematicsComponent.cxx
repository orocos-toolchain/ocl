/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  KinematicsComponent.cxx 

                        KinematicsComponent.cxx -  description
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
 
 

#include <kinematics/KinematicsComponent.hpp>
#include <rtt/Logger.hpp>

namespace Orocos
{
    using namespace KDL;
    using namespace RTT;
    
    KinematicsComponent::KinematicsComponent(const std::string& name,KinematicFamily* _kf)
        : TaskContext(name),
          kf(_kf),
          jnt2cartpos(_kf->createJnt2CartPos()),
          cartpos2jnt(_kf->createCartPos2Jnt()),
          jnt2jac(_kf->createJnt2Jac()),
          jnt2CartVel(_kf->createJnt2CartVel()),
          CartVel2Jnt(_kf->createCartVel2Jnt()),
          kinarch("Architecture", "The Kinematic Architecture name", _kf->getTypeName()),
          qPort("JointPositions"), 
          qdotPort("JointVelocities")
    {
        attributes()->addProperty(&kinarch);
        /*
        methods()->addMethod(method("setDelta", &ORO_KinDyn::KinematicsComponent::setDelta,
                                      "Change the delta radial movement this class can track "
                                      "inbetween calls to its interface. Defaults to 0.5 radians.",
                                      "deltaRad","Trackable change in position in radians"));

        methods()->addMethod("getDelta",method( &ORO_KinDyn::KinematicsComponent::getDelta,
                                      "Get the delta radial movement this class can track "
                                      "inbetween calls to its interface. Defaults to 0.5 radians."));
        */
        typedef KinematicsComponent KC;
        
        methods()->addMethod(method("setPlanningMode", &KC::setPlanningMode,
                                    "Use the kinematics component for planning, do not implicitly update the joint state."));
            
        methods()->addMethod(method("setTrackingMode", &KC::setTrackingMode,
                                    "Use the kinematics component for tracking, try to implicitly update the joint state."));

        methods()->addMethod(method("isPlanning", &KC::isPlanning,
                                    "Returns true if calculations do not implicitly update the joint state."));

        methods()->addMethod(method("isTracking", &KC::isTracking,
                                    "Returns true if calculations implicitly (try to) update the joint state."));
            
        methods()->addMethod(method("jacobianForward", &KC::jacobianForward,
                                    "Calculate the forward Jacobian at a given position.",
                                    "q", "Current position of the robot in radians.",
                                    "jac", "The resulting Jacobian at position q."));
        /*
        methods()->addMethod(method("jacobianInverse", &KC::jacobianInverse,
                                    "Calculate the inverse Jacobian at a given position.",
                                    "q", "Current position of the robot in radians.",
                                    "jac", "The resulting Jacobian at position q."));
        */
        
        methods()->addMethod(method("positionForward",  &KC::positionForward,
                                    "Calculate the end frame of the robot.",
                                    "q", "Current position of the robot in radians.",
                                    "mp_base", "The resulting endpoint frame at position q."));

        methods()->addMethod(method("positionInverse",  &KC::positionInverse,
                                    "Calculate the joint positions of the robot.",
                                    "mp_base", "The current endpoint frame of the robot.",
                                    "q", "Resulting position of the robot in radians."));

        methods()->addMethod(method("velocityForward",  &KC::velocityForward,
                                    "Calculate the end frame and end velocity of the robot.",
                                    "q", "Current position of the robot in radians.",
                                    "qdot", "Current velocity of the robot in radians/second.",
                                    "mp_base", "The resulting endpoint frame at position q.",
                                    "vel_base", "The resulting endpoint velocity at position q."));

        methods()->addMethod(method("velocityInverse",  &KC::velocityInverse,
                                    "Calculate the joint velocities of the robot.",
                                    "mp_base", "The current endpoint frame of the robot.",
                                    "vel_base", "The current endpoint velocity at position q.",
                                    "q", "Resulting position of the robot joints in radians.",
                                    "qdot", "Resulting velocity of the robot joints in radians/second."));

        methods()->addMethod(method("velocityInverse",  &KC::velocityInverse,
                                    "Calculate the joint velocities of the robot.",
                                    "q", "Current position of the robot in radians.",
                                    "vel_base", "The current endpoint velocity at position q.",
                                    "qdot", "Resulting velocity of the robot joints in radians/second."));

        methods()->addMethod(method("setPosition",  &KC::setPosition,
                                    "Update component state with new joint positions.",
                                    "qnew", "The new joint positions of the robot."));

        methods()->addMethod(method("getPosition",  &KC::getPosition,
                                    "Get current joint positions.",
                                    "qcur", "The current joint positions of the robot."));

        methods()->addMethod(method("getFrame",  &KC::getFrame,
                                    "Get current end effector frame."));
        
        methods()->addMethod(method("getTwist",  &KC::getTwist,
                                    "Get current end effector twist."));

        
    }

    const std::string& KinematicsComponent::getArchitecture() const {
        return kf->getTypeName();
    }

    KDL::* KinematicsComponent::getKinematics() {
        return &mykincomp;
    }

  void KinematicsComponent::setAlgorithm( ORO_KinDyn::KinematicsInterface *kin,
					  const ORO_KinDyn::JointVelocities& _signs /*= ORO_KinDyn::JointVelocities()*/,
					  const ORO_KinDyn::JointPositions&  _offsets /*= ORO_KinDyn::JointPositions()*/
					  )
  {
    mykin = kin;
    mykincomp.setKinematics( kin, _signs, _offsets );
  }

    bool KinematicsComponent::startup() {

        if ( kinarch.get() != "none" )
            {
                mykin = ORO_KinDyn::KinematicsFactory::Create( kinarch );
                if (mykin) {
                    mykincomp.setKinematics( mykin );
                    tmppos.reserve( mykin->maxNumberOfJoints() );
                    tmppos.resize( mykin->getNumberOfJoints() );
                    return true;
                }
                Logger::log() << Logger::Error << "'"<< kinarch
                              << "' kinematic architecture not found."<<Logger::endl;
                return false;
                        
            }
        if ( mykin == 0 ) {
            Logger::log() << Logger::Error << "KinematicsComponent: No kinematic architecture set."<<Logger::endl;
            return false;
        }
        return true;
    }

    void KinematicsComponent::update() 
    {
        // read in joint pos and vel, if any.
        if (qPort.connected() && mykin) {
             tmppos = qPort.Get( );
            mykincomp.setPosition(tmppos);
        }
        if (qdotPort.connected() && mykin) {
             tmppos = qdotPort.Get( );
            mykincomp.setVelocity(tmppos);
        }
    }

}
