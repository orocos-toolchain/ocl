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


#include "Kuka361DWH.hpp"
#include <math.h>

#define EPSILON 0.00001
#define M_PI_T2 M_PI*2
#define SQRT3d2 0.8660254037844386
#define SQRT3t2 3.46410161513775

namespace OCL{
namespace Kuka361DWH{
    
    void convertGeometric(const std::vector<double>& q_hw,const std::vector<double>& qdot_sw,std::vector<double>& q_sw, std::vector<double>& qdot_hw)
    {
        q_sw[0]=-q_hw[0];
        q_sw[1]=q_hw[1];
        q_sw[2]=q_hw[2];
        
        qdot_hw[0]=-qdot_sw[0];
        qdot_hw[1]=qdot_sw[1];
        qdot_hw[2]=qdot_sw[2];


        // convert last 3 axis from DWH into ZXZ
        double c5 = cos(q_hw[4]);
        double s5 = sin(q_hw[4]);
        double c5_eq = (c5+3.)/4;   /* eq.(3-1) inverse */
        double alpha;

        if (q_hw[4]<-EPSILON){
            alpha = atan2(-s5,SQRT3d2*(c5-1.));  /* eq.(3-3)/(3-4) */
            q_sw[4]=-2.*acos(c5_eq);
            qdot_hw[ 4 ]=-sqrt((1.-c5)*(7.+c5))*qdot_sw[4]/2./s5;
        }else{
            if (q_hw[4]<EPSILON){
                alpha = M_PI_2;
                q_sw[4] = 0.0;
                qdot_hw[4]=qdot_sw[4];
            }else{
                alpha = atan2( s5, SQRT3d2 * ( 1. - c5 ) );
                q_sw[4] = 2.*acos(c5_eq);
                qdot_hw[4] = sqrt((1.-c5)*(7.+c5))*qdot_sw[4]/2./s5;
            }
        }

        q_sw[ 3 ] = -q_hw[ 3 ] + alpha;
        q_sw[ 5 ] = -q_hw[ 5 ] - alpha;

        double alphadot = -SQRT3t2/(7.+c5)*qdot_sw[4];

        qdot_hw[3] = -qdot_sw[3]-alphadot;
        qdot_hw[5] = -qdot_sw[5]+alphadot;

    }
    
}
}
    
