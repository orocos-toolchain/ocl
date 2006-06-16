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
 
 

#include "KinematicsComponent.hpp"
#include "corelib/Logger.hpp"

namespace Orocos
{
    using namespace ORO_KinDyn;
    using namespace RTT;
    KinematicsComponent::KinematicsComponent(const std::string& name)
        : GenericTaskContext(name),
          kinarch("Architecture", "The Kinematic Architecture name, for example, Kuka160 or Kuka361.", "none"),
          mykin(0),
          mykincomp(0),
          qPort("JointPositions"), 
          qdotPort("JointVelocities")
    {
        attributes()->addProperty(&kinarch);
    }

    const std::string& KinematicsComponent::getArchitecture() const {
        return kinarch.rvalue();
    }

    ORO_KinDyn::KinematicsComponent* KinematicsComponent::getKinematics() {
        return &mykincomp;
    }

    void KinematicsComponent::setAlgorithm( ORO_KinDyn::KinematicsInterface *kin,
                                         const ORO_KinDyn::JointVelocities& _signs /*= ORO_KinDyn::JointVelocities()*/, 
                                         const ORO_KinDyn::JointPositions&  _offsets /*= ORO_KinDyn::JointPositions()*/ ) {
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

    ORO_Execution::MethodFactoryInterface* KinematicsComponent::createMethodFactory()
    {
        TemplateMethodFactory<ORO_KinDyn::KinematicsComponent>* kfact =
            new TemplateMethodFactory<ORO_KinDyn::KinematicsComponent>(&mykincomp);

        kfact->add("setDelta",method( &ORO_KinDyn::KinematicsComponent::setDelta,
                                      "Change the delta radial movement this class can track "
                                      "inbetween calls to its interface. Defaults to 0.5 radians.",
                                      "deltaRad","Trackable change in position in radians"));
        kfact->add("getDelta",method( &ORO_KinDyn::KinematicsComponent::getDelta,
                                      "Get the delta radial movement this class can track "
                                      "inbetween calls to its interface. Defaults to 0.5 radians."));

        kfact->add("setPlanningMode",method( &ORO_KinDyn::KinematicsComponent::setPlanningMode,
                                             "Use the kinematics component for planning, do not implicitly update the joint state."));
            
        kfact->add("setTrackingMode",method( &ORO_KinDyn::KinematicsComponent::setTrackingMode,
                                             "Use the kinematics component for tracking, try to implicitly update the joint state."));
        kfact->add("isPlanning",method( &ORO_KinDyn::KinematicsComponent::isPlanning,
                                        "Returns true if calculations do not implicitly update the joint state."));
        kfact->add("isTracking",method( &ORO_KinDyn::KinematicsComponent::isTracking,
                                        "Returns true if calculations implicitly (try to) update the joint state."));
            
        kfact->add("jacobianForward",method( &ORO_KinDyn::KinematicsComponent::jacobianForward,
                                             "Calculate the forward Jacobian at a given position.",
                                             "q", "Current position of the robot in radians.",
                                             "jac", "The resulting Jacobian at position q."));
        kfact->add("jacobianInverse",method( &ORO_KinDyn::KinematicsComponent::jacobianInverse,
                                             "Calculate the inverse Jacobian at a given position.",
                                             "q", "Current position of the robot in radians.",
                                             "jac", "The resulting Jacobian at position q."));
            
        kfact->add("positionForward", method( &ORO_KinDyn::KinematicsComponent::positionForward,
                                              "Calculate the end frame of the robot.",
                                              "q", "Current position of the robot in radians.",
                                              "mp_base", "The resulting endpoint frame at position q."));

        kfact->add("positionInverse", method( &ORO_KinDyn::KinematicsComponent::positionInverse,
                                              "Calculate the joint positions of the robot.",
                                              "mp_base", "The current endpoint frame of the robot.",
                                              "q", "Resulting position of the robot in radians."));

        kfact->add("velocityForward", method( &ORO_KinDyn::KinematicsComponent::velocityForward,
                                              "Calculate the end frame and end velocity of the robot.",
                                              "q", "Current position of the robot in radians.",
                                              "qdot", "Current velocity of the robot in radians/second.",
                                              "mp_base", "The resulting endpoint frame at position q.",
                                              "vel_base", "The resulting endpoint velocity at position q."));

        kfact->add("velocityInverse", method( &ORO_KinDyn::KinematicsComponent::velocityInverse,
                                              "Calculate the joint velocities of the robot.",
                                              "mp_base", "The current endpoint frame of the robot.",
                                              "vel_base", "The current endpoint velocity at position q.",
                                              "q", "Resulting position of the robot joints in radians.",
                                              "qdot", "Resulting velocity of the robot joints in radians/second."));
        kfact->add("velocityInverse", method( &ORO_KinDyn::KinematicsComponent::velocityInverse,
                                              "Calculate the joint velocities of the robot.",
                                              "q", "Current position of the robot in radians.",
                                              "vel_base", "The current endpoint velocity at position q.",
                                              "qdot", "Resulting velocity of the robot joints in radians/second."));

        kfact->add("setPosition", method( &ORO_KinDyn::KinematicsComponent::setPosition,
                                          "Update component state with new joint positions.",
                                          "qnew", "The new joint positions of the robot."));
        kfact->add("getPosition", method( &ORO_KinDyn::KinematicsComponent::getPosition,
                                          "Get current joint positions.",
                                          "qcur", "The current joint positions of the robot."));
        kfact->add("getFrame", method( &ORO_KinDyn::KinematicsComponent::getFrame,
                                       "Get current end effector frame."));
        kfact->add("getTwist", method( &ORO_KinDyn::KinematicsComponent::getTwist,
                                       "Get current end effector twist."));

        return kfact;
    }

}
