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

/*Implementation based on ForPosZXXDWH from rtt-branch-0.24 : which
 * is copyrighted by Peter Soetens */


#include "Kuka361Kinematics.hpp"

#define M_PI_T2 M_PI*2

namespace OCL{
    using namespace KDL;

    void Kuka361Kinematics::calc_eq(const JntArray& q,JntArray& q_eq)
    {
        /* We first calculate the joint values of the `equivalent' robot.
           Since only the wrist of both robots differs, only joints 4, 5 and 6
           have to be compensated (eq.(3.1)-(3.8), Willems and Asselberghs.
        */
       
        for(unsigned int i=0;i<3;i++)
            q_eq(i)=q(i);
        
        double c5 = cos( q( 4 ) );
        double s5 = sin( q( 4 ) );
        double c5_eq = ( c5 + 3. ) / 4;   /* eq.(3-1) inverse */
        
        double alpha;
        if ( q( 4 ) < -epsilon ){
            alpha = atan2( -s5, 0.8660254037844386 * ( c5 - 1. ) );  /* eq.(3-3)/(3-4) */
            q_eq( 4 ) = -2. * acos( c5_eq );
        }else{
            if ( q( 4 ) < epsilon ){
                alpha = M_PI_2;
                q_eq( 4 ) = 0.0;
            }else{
                alpha = atan2( s5, 0.8660254037844386 * ( 1. - c5 ) );
                q_eq( 4 ) = 2. * acos( c5_eq );
            }
        }
        
        q_eq( 3 ) = q( 3 ) + alpha;
        q_eq( 5 ) = q( 5 ) - alpha;
    }

    void Kuka361Kinematics::calc_eq(const JntArrayVel& q,JntArrayVel& q_eq)
    {
        this->calc_eq(q.q,q_eq.q);
        /* We first calculate the joint values of the `equivalent' robot.
           Since only the wrist of both robots differs, only joints 4, 5 and 6
           have to be compensated (eq.(3.1)-(3.8), Willems and Asselberghs.
        */
        for(unsigned int i=0;i<3;i++)
            q_eq.qdot(i)=q.qdot(i);


        double c5 = cos( q.q( 4 ) );
        double s5 = sin( q.q( 4 ) );
        
        if ( q.q( 4 ) < -epsilon ){
            q_eq.qdot( 4 ) = -2. * s5 * q.qdot( 4 ) / sqrt( ( 1. - c5 ) * ( c5 + 7. ) );
        }else{
            if ( q.q( 4 ) < epsilon ){
                q_eq.qdot( 4 ) = q.qdot( 4 );
            }else{
                q_eq.qdot( 4 ) = 2. * s5 * q.qdot( 4 ) / sqrt( ( 1. - c5 ) * ( c5 + 7. ) );
            }
        }

        double alphadot = -3.46410161513775 / ( 7. + c5 ) * q.qdot( 4 );
        q_eq.qdot( 3 ) = q.qdot( 3 ) + alphadot;
        q_eq.qdot( 5 ) = q.qdot( 5 ) - alphadot;
    }
    
    void Kuka361Kinematics::calc_eq_inv(const JntArray& q_eq,JntArray& q)
    {
        for(unsigned int i=0;i<3;i++)
            q(i)=q_eq(i);
        
        double c5 = 4. * cos ( q_eq( 4 ) / 2. ) - 3.;
        
        if (c5 > 1.) c5 = 1; else if (c5 <-1.) c5=-1.; // correction for rounding errors
        
        double alpha;
        
        if ( q_eq( 4 ) < -epsilon ){
            q( 4 ) = -acos( c5 );
            double s5 = sin( q( 4 ) );
            alpha = atan2 ( -s5, 0.8660254037844386 * ( c5 - 1. ) );
        }else{
            if ( q_eq( 4 ) < epsilon ){
                alpha = M_PI_2;
                q( 4 ) = 0.0;
            }else{
                q( 4 ) = acos( c5 );
                double s5 = sin( q( 4 ) );
                alpha = atan2 ( s5, 0.8660254037844386 * ( 1. - c5 ) );
            }
        }
        
        q( 3 ) = q_eq( 3 ) - alpha;
        
        if ( q( 3 ) >= M_PI )
            q( 3 ) -= M_PI_T2;
        else if ( q( 3 ) <= -M_PI )
            q( 3 ) += M_PI_T2;

        q( 5 ) = q_eq( 5 ) + alpha;

        if ( q( 5 ) >= M_PI )
            q( 5 ) -= M_PI_T2;
        else if ( q( 5 ) <= -M_PI )
            q( 5 ) += M_PI_T2;
        
    }

    void Kuka361Kinematics::calc_eq_inv(const JntArrayVel& q_eq,JntArrayVel& q)
    {
        this->calc_eq_inv(q_eq.q,q.q);
        /* We first calculate the joint values of the `equivalent' robot.
           Since only the wrist of both robots differs, only joints 4, 5 and 6
           have to be compensated (eq.(3.1)-(3.8), Willems and Asselberghs.
        */

        double s5 = sin( q_eq.q( 4 ) );
        double c5 = 4. * cos ( q_eq.q( 4 ) / 2. ) - 3.;

        if ( q_eq.q( 4 ) < -epsilon )
        {
            q.qdot( 4 ) = -sqrt( ( 1. - c5 ) * ( 7. + c5 ) ) * q_eq.qdot( 4 ) / 2. / s5;
        }

        else
        {
            if ( q_eq.q( 4 ) < epsilon )
            {
                q.qdot( 4 ) = q_eq.qdot( 4 );
            }

            else
            {
                q.qdot( 4 ) = sqrt( ( 1. - c5 ) * ( 7. + c5 ) ) * q_eq.qdot( 4 ) / 2. / s5;
            }
        }
        double alphadot = -3.46410161513775 / ( 7. + c5 ) * q.qdot( 4 );

        q.qdot( 3 ) = q_eq.qdot( 3 ) - alphadot;
        q.qdot( 5 ) = q_eq.qdot( 5 ) + alphadot;
        for(unsigned int i=0;i<3;i++)
            q.qdot(i)=q_eq.qdot(i);
        
    }
    
        
    
    int Kuka361Kinematics::JntToCart(const JntArray& q, Frame& p_out,int segmentNr)
    {
        //This implementation does not support intermediate frames
        if(segmentNr!=-1)
            return -1;
        
        /* Compensation factors and compensated joint values for the wrist: */
        JntArray q_eq(6);
        this->calc_eq(q,q_eq);
        

        /* From here on, the kinematics are identical to those of the ZXXZXZ: */
        double c1 = cos( q_eq( 0 ) );
        double s1 = sin( q_eq( 0 ) );
        double c23 = cos( q_eq( 1 ) + q_eq( 2 ) );
        double s23 = sin( q_eq( 1 ) + q_eq( 2 ) );
        double c4 = cos( q_eq( 3 ) );
        double s4 = sin( q_eq( 3 ) );
        double c5 = cos( q_eq( 4 ) );
        double s5 = sin( q_eq( 4 ) );
        double c6 = cos( q_eq( 5 ) );
        double s6 = sin( q_eq( 5 ) );
        double s5c6 = s5 * c6;
        double s5s6 = s5 * s6;
        double c4s5 = c4 * s5;
        double s4s5 = s4 * s5;
        double c1c23 = c1 * c23;
        double c1s23 = c1 * s23;
        double s1c23 = s1 * c23;
        double s1s23 = s1 * s23;

        p_out.M( 1, 0 ) = s4 * c5 * s6 - c4 * c6;
        p_out.M( 2, 0 ) = c4 * c5 * s6 + s4 * c6;
        p_out.M( 0, 0 ) = -s1c23 * p_out.M( 2, 0 ) - c1 * p_out.M( 1, 0 ) + s1s23 * s5s6;
        p_out.M( 1, 0 ) = c1c23 * p_out.M( 2, 0 ) - s1 * p_out.M( 1, 0 ) - c1s23 * s5s6;
        p_out.M( 2, 0 ) = - s23 * p_out.M( 2, 0 ) - c23 * s5s6;

        p_out.M( 1, 1 ) = s4 * c5 * c6 + c4 * s6;
        p_out.M( 2, 1 ) = c4 * c5 * c6 - s4 * s6;
        p_out.M( 0, 1 ) = -s1c23 * p_out.M( 2, 1 ) - c1 * p_out.M( 1, 1 ) + s1s23 * s5c6;
        p_out.M( 1, 1 ) = c1c23 * p_out.M( 2, 1 ) - s1 * p_out.M( 1, 1 ) - c1s23 * s5c6;
        p_out.M( 2, 1 ) = - s23 * p_out.M( 2, 1 ) - c23 * s5c6;

        p_out.M( 0, 2 ) = -s1c23 * c4s5 - c1 * s4s5 - s1s23 * c5;
        p_out.M( 1, 2 ) = c1c23 * c4s5 - s1 * s4s5 + c1s23 * c5;
        p_out.M( 2, 2 ) = - s23 * c4s5 + c23 * c5;

        /* vertical distance of wrist from shoulder: */
        double dWv = cos( q_eq( 1 ) ) * l2 + c23 * l3;
        /* horizontal distance of wrist from shoulder: */
        double dWh = sin( q_eq( 1 ) ) * l2 + s23 * l3;

        /* Position of last link: */
        Vector P6 = p_out.M.UnitZ()*l6;
        
        /* Position of wrist: */
        Vector Pw(- s1 * dWh, c1 * dWh, l1 + dWv);

        /* End effector position = position of last link + position of wrist: */
        p_out.p = P6+Pw;

        return 0;

    }

    int Kuka361Kinematics::JntToCart(const JntArrayVel& q_in, FrameVel& out,int segmentNr)
    {
        //This implementation does not support intermediate frames
        if(segmentNr!=-1)
            return -1;
        
        JntArrayVel q_eq( 6 );
        this->calc_eq(q_in,q_eq);
        
        double c1 = cos( q_eq.q( 0 ) );
        double s1 = sin( q_eq.q( 0 ) );
        double c23 = cos( q_eq.q( 1 ) + q_eq.q( 2 ) );
        double s23 = sin( q_eq.q( 1 ) + q_eq.q( 2 ) );
        double c4 = cos( q_eq.q( 3 ) );
        double s4 = sin( q_eq.q( 3 ) );
        double c5 = cos( q_eq.q( 4 ) );
        double s5 = sin( q_eq.q( 4 ) );
        double c4s5 = c4 * s5;
        double s4s5 = s4 * s5;

        Frame pos;
        Twist vel;
        this->JntToCart(q_in.q,pos);

        /* vertical distance of wrist from shoulder: */
        double dWv = cos( q_eq.q( 1 ) ) * l2 + c23 * l3;
        /* horizontal distance of wrist from shoulder: */
        double dWh = sin( q_eq.q( 1 ) ) * l2 + s23 * l3;

        /* Position of wrist: */
        Vector Pw(- s1 * dWh, c1 * dWh, l1 + dWv);

        /* Forward velocity transformation according to Featherstone eqs.(37)-(43) */
        double Wwy = -s4 * q_eq.qdot( 4 ) + c4s5 * q_eq.qdot( 5 );
        double Wwz = q_eq.qdot( 3 ) + c5 * q_eq.qdot( 5 );
        
        double Wx = -c4 * q_eq.qdot( 4 ) - s4s5 * q_eq.qdot( 5 ) - q_eq.qdot( 1 ) - q_eq.qdot( 2 );
        double Wy = c23 * Wwy + s23 * Wwz;
        
        vel.rot.x(c1 * Wx - s1 * Wy);
        vel.rot.y(c1 * Wy + s1 * Wx);
        vel.rot.z(q_eq.qdot( 0 ) + c23 * Wwz - s23 * Wwy);

        
        double Vx = -dWh * q_eq.qdot( 0 );
        double Vy = dWv * q_eq.qdot( 1 ) + l3 * c23 * q_eq.qdot( 2 );

        vel.vel.x(c1 * Vx - s1 * Vy - vel.rot.y()*Pw.z() + vel.rot.z()*Pw.y());
        vel.vel.y(c1 * Vy + s1 * Vx - vel.rot.z()*Pw.x() + vel.rot.x()*Pw.z());
        vel.vel.z(-dWh * q_eq.qdot( 1 ) - l3 * s23 * q_eq.qdot( 2 ) 
                  - vel.rot.x()*Pw.y() + vel.rot.y()*Pw.x());

        out=FrameVel(pos,vel);
        
        return 0;
    }

    int Kuka361Kinematics::CartToJnt(const JntArray& q_in, const Twist& v_in, JntArray& qdot_out)
    {
        
        /* We first calculate the joint values of the `equivalent' robot.
           Since only the wrist of both robots differs, only joints 4, 5 and 6
           have to be compensated (eq.(3.1)-(3.8), Willems and Asselberghs.
           */
        JntArrayVel q_eq(6);
        JntArrayVel q(6);

        calc_eq(q_in,q_eq.q);
                

        /* From here on, the kinematics are identical to those of the ZXXZXZ: */
        double c1 = cos( q_eq.q( 0 ) );
        double s1 = sin( q_eq.q( 0 ) );
        double s3 = sin( q_eq.q( 2 ) );
        double c23 = cos( q_eq.q( 1 ) + q_eq.q( 2 ) );
        double s23 = sin( q_eq.q( 1 ) + q_eq.q( 2 ) );
        double c4 = cos( q_eq.q( 3 ) );
        double s4 = sin( q_eq.q( 3 ) );
        double c5 = cos( q_eq.q( 4 ) );
        double s5 = sin( q_eq.q( 4 ) );
        
        /* vertical distance of wrist from shoulder: */
        double dWv = cos( q_eq.q( 1 ) ) * l2 + c23 * l3;
        /* horizontal distance of wrist from shoulder: */
        double dWh = sin( q_eq.q( 1 ) ) * l2 + s23 * l3;

        /* Position of wrist: */
        Vector Pw( - s1 * dWh, c1 * dWh, l1 + dWv);

        //************************************************************************************
        //************************************************************************************
        //   NO BUG UNTIL HERE, q_eq is equal with long algorithm.
        //************************************************************************************
        //************************************************************************************
        //************************************************************************************
        /* Inverse velocity calculations: */

        /* Calculation of wrist velocity: Low & Dubey, eq.(47) */
        Vector Vw(v_in.vel.x() + v_in.rot.y() * Pw.z() - v_in.rot.z() * Pw.y(),
                  v_in.vel.y() + v_in.rot.z() * Pw.x() - v_in.rot.x() * Pw.z(),
                  v_in.vel.z() + v_in.rot.x() * Pw.y() - v_in.rot.y() * Pw.x());
        

        /* First, second and third joint velocities: eqs.(44)-(45), Low & Dubey,
           eqs.(6.24),(6.25) Brochier & Haverhals */
        q_eq.qdot( 0 ) = -( c1 * Vw.x() + s1 * Vw.y() ) / dWh;

        double temp3 = c1 * Vw.y() - s1 * Vw.x();

        q_eq.qdot( 1 ) = ( s23 * temp3 + c23 * Vw.z() ) / l2 / s3;

        q_eq.qdot( 2 ) = -( dWh * temp3 + dWv * Vw.z() ) / l2 / l3 / s3;

        /* Velocity of end effector wrt the wrist: */
        Vector Ww;
        Ww.x( c1 * v_in.rot.y() - s1 * v_in.rot.x());
        Ww.y(q_eq.qdot( 1 ) + q_eq.qdot( 2 ) + c1 * v_in.rot.x() + s1 * v_in.rot.y());
        Ww.z(c23 * Ww.x() - s23 * ( v_in.rot.z() - q_eq.qdot( 0 ) ));

        /* Fourth, fifth and sixth joint velocities: eqs.(48)-(50) Low & Dubey */
        q_eq.qdot( 4 ) = -c4 * Ww.y() - s4 * Ww.z();
        q_eq.qdot( 5 ) = ( c4 * Ww.z() - s4 * Ww.y() ) / s5;
        q_eq.qdot( 3 ) = c23 * ( v_in.rot.z() - q_eq.qdot( 0 ) ) + s23 * Ww.x() - c5 * q_eq.qdot( 5 );

        /* This far, the `equivalent' robot's inverse velocity kinematics have been
           calculated. Now, we determine the correction factors `alpha' and `alphadot'
           for the fourth and sixth joint positions and velocities.
           (eqs. (3.1) -(3.8), Willems and Asselberghs). 
           `alphadot' is the time derivative of `alpha'. */
        calc_eq_inv(q_eq,q);
        qdot_out=q.qdot;
                    
        return 0;
    };

    int Kuka361Kinematics::CartToJnt(const JntArray& q_init, const FrameVel& v_in, JntArrayVel& q_out)
    {
        return 0;
    };

    int Kuka361Kinematics::JntToJac(const JntArray& q,Jacobian& J)
    {

        double c5 = cos( q( 4 ) );
        double s5 = sin( q( 4 ) );
        
        JntArray q_eq(6);
        calc_eq(q,q_eq);
        
        double A;
        if ( q( 4 ) < -epsilon ){
            A = -2. * s5 / sqrt( ( 1. - c5 ) * ( c5 + 7. ) );
        }else{
            if ( q( 4 ) < epsilon ){
                A = 1.;
            }else{
                A = 2. * s5 / sqrt( ( 1. - c5 ) * ( c5 + 7. ) );
            }
        }

        double B = 3.46410161513775 / ( 7. + c5 );
        
        double c1 = cos( q_eq( 0 ) );
        double s1 = sin( q_eq( 0 ) );
        double c23 = cos( q_eq( 1 ) + q_eq( 2 ) );
        double s23 = sin( q_eq( 1 ) + q_eq( 2 ) );
        double c4 = cos( q_eq( 3 ) );
        double s4 = sin( q_eq( 3 ) );
        c5 = cos( q_eq( 4 ) );
        s5 = sin( q_eq( 4 ) );
        double c4s5 = c4 * s5;
        double s4s5 = s4 * s5;
        double c1c23 = c1 * c23;
        double c1s23 = c1 * s23;
        double s1c23 = s1 * c23;
        double s1s23 = s1 * s23;

        double dWv = cos( q_eq( 1 ) ) * l2 + c23 * l3;
        double dWh = sin( q_eq( 1 ) ) * l2 + s23 * l3;

        Vector Pw(-s1 * dWh, c1 * dWh, l1 + dWv);
                    
        /* Featherstone, eq.(37)-(43) */
        /* Angular velocity components: */
        J( 3 , 0 ) = J( 4 , 0 ) = J( 5 , 1 ) = J( 5 , 2 ) = 0.;

        J( 3 , 1 ) = J( 3 , 2 ) = - c1;
        J( 3 , 3 ) = - s1s23;
        J( 3 , 4 ) = - c1 * c4 + s1c23 * s4;
        J( 3 , 5 ) = - s1 * ( c23 * c4s5 + s23 * c5 ) - c1 * s4s5;

        J( 4 , 1 ) = J( 4 , 2 ) = - s1;
        J( 4 , 3 ) = c1s23;
        J( 4 , 4 ) = - s1 * c4 - c1c23 * s4;
        J( 4 , 5 ) = - s1 * s4s5 + c4s5 * c1c23 + c1s23 * c5;

        J( 5 , 0 ) = 1.;
        J( 5 , 3 ) = c23;
        J( 5 , 4 ) = s23 * s4;
        J( 5 , 5 ) = c23 * c5 - s23 * c4s5;

        /* Linear velocity components: */
        J( 0 , 0 ) = - c1 * dWh + Pw.y();
        J( 0 , 1 ) = J( 0 , 2 ) = s1 * Pw.z();
        J( 0 , 1 ) -= s1 * dWv;
        J( 0 , 2 ) -= s1c23 * l3;
        J( 0 , 3 ) = J( 5 , 3 ) * Pw.y() - J( 4 , 3 ) * Pw.z();
        J( 0 , 4 ) = J( 5 , 4 ) * Pw.y() - J( 4 , 4 ) * Pw.z();
        J( 0 , 5 ) = J( 5 , 5 ) * Pw.y() - J( 4 , 5 ) * Pw.z();

        J( 1 , 0 ) = -s1 * dWh - Pw.x();
        J( 1 , 1 ) = J( 1 , 2 ) = -c1 * Pw.z();
        J( 1 , 1 ) += c1 * dWv;
        J( 1 , 2 ) += c1c23 * l3;
        J( 1 , 3 ) = J( 3 , 3 ) * Pw.z() - J( 5 , 3 ) * Pw.x();
        J( 1 , 4 ) = J( 3 , 4 ) * Pw.z() - J( 5 , 4 ) * Pw.x();
        J( 1 , 5 ) = J( 3 , 5 ) * Pw.z() - J( 5 , 5 ) * Pw.x();

        J( 2 , 0 ) = 0.;
        J( 2 , 1 ) = J( 2 , 2 ) = c1 * Pw.y() - s1 * Pw.x();
        J( 2 , 1 ) -= dWh;
        J( 2 , 2 ) -= s23 * l3;
        J( 2 , 3 ) = J( 4 , 3 ) * Pw.x() - J( 3 , 3 ) * Pw.y();
        J( 2 , 4 ) = J( 4 , 4 ) * Pw.x() - J( 3 , 4 ) * Pw.y();
        J( 2 , 5 ) = J( 4 , 5 ) * Pw.x() - J( 3 , 5 ) * Pw.y();

        /* Compensations for fourth, fifth and sixth rows of `J': */

        for ( unsigned int i = 0;i < 6;i++ )
        {
            J( i , 4 ) = A * J( i , 4 ) + B * ( -J( i , 3 ) + J( i , 5 ) );
        }

        return 0;
    }
}

    
