// Copyright  (C)  2008  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// Maintainer: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <kdl/chainfksolver.hpp>
#include <kdl/chainiksolver.hpp>

namespace OCL{
    using namespace KDL;
    
    #define KUKA361_L1 1.020
    #define KUKA361_L2 0.480
    #define KUKA361_L3 0.645
    #define KUKA361_L6 0.120
    
    class Kuka361Kinematics : public ChainFkSolverPos, 
                              public ChainFkSolverVel, 
                              public ChainIkSolverVel//, 
                              //public ChainIkSolverPos 
    {
    public:
        Kuka361Kinematics():l1(KUKA361_L1),l2(KUKA361_L2),l3(KUKA361_L3),l6(KUKA361_L6)
            {};
        ~Kuka361Kinematics(){};
        
        virtual int JntToCart(const JntArray& q_in, Frame& p_out,int segmentNr=-1);
        virtual int JntToCart(const JntArrayVel& q_in, FrameVel& out,int segmentNr=-1);
        
        virtual int CartToJnt(const JntArray& q_in, const Twist& v_in, JntArray& qdot_out);
        virtual int CartToJnt(const JntArray& q_init, const FrameVel& v_in, JntArrayVel& q_out);

        int JntToJac(const JntArray& q, Jacobian& J);
        
    private:
        double l1,l2,l3,l6;
        void calc_eq(const JntArray& q_in,JntArray& q_out);
        void calc_eq(const JntArrayVel& q_in, JntArrayVel& q_out);
        void calc_eq_inv(const JntArray& q_in,JntArray& q_out);
        void calc_eq_inv(const JntArrayVel& q_in, JntArrayVel& q_out);
        
    };
}//end of namespace

        
