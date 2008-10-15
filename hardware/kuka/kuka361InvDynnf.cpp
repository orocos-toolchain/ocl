
#include "kuka361InvDynnf.hpp"
#include <iostream>

#define FSIGN(a) ( a<-1E-15 ? -1.0 : ( a>1E-15 ? 1.0 : 0.0))
#define SSIGN2(a,b) (2.0/(1.0+exp(-3*a/b))-1.0)

//namespace Orocos
//{
using namespace std;

#define Y_LS {29.0130, -10.9085, -0.2629, 2.6403, -0.0421, 0.0509, 3.0221, 2.4426, 0.0531, -0.2577, -0.2627, 346.7819, -0.2651, 35.4666, 5.6321, 0.9935, 3.2192, 10.7823, 34.2604, 16.9902, 39.1594, 5.4501, 16.1392, -12.7243, -6.1094, 1.9590}
#define Y_LS2 {3.4102, 3.2555, 2.2401, 29.5178, 35.0201, 28.1370, 16.3857, 15.5666, 14.5318, -0.5734, -2.1976, 5.8708} 

kuka361InvDynnf::kuka361InvDynnf(double dqm_):_tau(6),dqm(dqm_){
	D13 = 0.48;
	g1 = 9.81;
	double y_ls[26] = Y_LS;
	for(int i=0; i<26;i++){
		_y_ls[i]=y_ls[i];
	}
	r3 = 51.44118; 
	l = 0.488;
	r = 0.1;
	double y_ls2[12] = Y_LS2;
	for(int i=0; i<12;i++){
		_y_ls2[i]=y_ls2[i];
	}
}

vector<double> kuka361InvDynnf::invdyn361(vector<double> &q, vector<double> &dq, vector<double> &dq_des, vector<double> &ddq, vector<double> &torque_scale, vector<double> &torque_offset){
	//Calculate ddq of fist three axes

// 	printf("q1: %f \n",q[0]);
// 	printf("q2: %f \n",q[1]);
// 	printf("q3: %f \n",q[2]);
// 	printf("q4: %f \n",q[3]);
// 	printf("q5: %f \n",q[4]);
// 	printf("q6: %f \n",q[5]);
// 	
// 	printf("dq1: %f \n",dq[0]);
// 	printf("dq2: %f \n",dq[1]);
// 	printf("dq3: %f \n",dq[2]);
// 	printf("dq4: %f \n",dq[3]);
// 	printf("dq5: %f \n",dq[4]);
// 	printf("dq6: %f \n",dq[5]);
// 	
// 	printf("ddq1: %f \n",ddq[0]);
// 	printf("ddq2: %f \n",ddq[1]);
// 	printf("ddq3: %f \n",ddq[2]);
// 	printf("ddq4: %f \n",ddq[3]);
// 	printf("ddq5: %f \n",ddq[4]);
// 	printf("ddq6: %f \n",ddq[5]);   

	t1 = dq[1] * dq[1];
	t3 = q[1] + q[2];
	t4 = sin(t3);
	t6 = _y_ls[7] * dq[0];
	t8 = cos(t3);
	t12 = dq[2] * t4;
	t16 = _y_ls[2] * dq[0];
	t17 = cos(q[1]);
	t18 = t17 * t17;
	t30 = _y_ls[5] * dq[0];
	t31 = t30 * dq[1];
	t32 = D13 * t17;
	t33 = t32 * t8;
	t36 = _y_ls[6] * dq[0];
	t37 = t36 * dq[1];
	t38 = sin(q[1]);
	t39 = D13 * t38;
	t40 = t39 * t8;
	t43 = t39 * t4;
	t49 = t32 * t4;
	t52 = _y_ls[8] * dq[0];
	t53 = t8 * t8;
	t57 = -_y_ls[10] * t1 * t4 + 0.4e1 * t6 * dq[1] * t4 * t8 + 0.4e1 * t6 * t12 * t8 - 0.4e1 * t16 * dq[1] * t18 + SSIGN2(dq_des[0],dqm) * _y_ls[18] + 0.2e1 * _y_ls[9] * dq[1] * dq[2] * t8 - 0.2e1 * _y_ls[10] * dq[1] * t12 - 0.2e1 * t31 * t33 + 0.2e1 * t37 * t40 + 0.2e1 * t31 * t43 + 0.2e1 * t30 * dq[2] * t43 + 0.2e1 * t37 * t49 + 0.4e1 * t52 * dq[1] * t53;
	t71 = dq[2] * dq[2];
	t87 = _y_ls[23] + 0.4e1 * t52 * dq[2] * t53 + dq[0] * _y_ls[17] + 0.2e1 * t36 * dq[2] * t40 - 0.2e1 * t52 * dq[2] + 0.2e1 * t16 * dq[1] - 0.2e1 * t52 * dq[1] - _y_ls[10] * t71 * t4 - _y_ls[4] * t1 * t17 + _y_ls[9] * t1 * t8 + _y_ls[9] * t71 * t8 - _y_ls[3] * t1 * t38 - 0.4e1 * _y_ls[1] * dq[0] * dq[1] * t17 * t38;
	tau1 = t57 + t87;
	t88 = l * l;
	t89 = r * r;
	t90 = l * r;
	t94 = sqrt(t88 + t89 - 0.2e1 * t90 * t17);
	t99 = dq[0] * dq[0];
	t105 = t99 * t4;
	t106 = t8 * _y_ls[7];
	t110 = _y_ls[5] * t94;
	t111 = t110 * t99;
	t114 = dq[2] * D13;
	t115 = cos(q[2]);
	t119 = g1 * t17;
	t122 = g1 * t38;
	t123 = sin(q[2]);
	t126 = _y_ls[6] * t94;
	t127 = t126 * t99;
	t134 = t71 * D13;
	t137 = -_y_ls[24] * t94 + 0.1000e4 * t90 * t38 * _y_ls[16] - 0.2e1 * t99 * t17 * t38 * _y_ls[1] * t94 + 0.2e1 * t105 * t106 * t94 + t111 * t43 - 0.2e1 * t110 * dq[1] * t114 * t115 - t110 * t119 * t115 + t110 * t122 * t123 + t127 * t49 + t127 * t40 + 0.2e1 * t126 * dq[1] * t114 * t123 + t126 * t134 * t123;
	t150 = t99 * _y_ls[2];
	t155 = t99 * _y_ls[8];
	t163 = t126 * t119 * t123 + t126 * t122 * t115 - t38 * _y_ls[11] * t94 - dq[1] * _y_ls[19] * t94 - SSIGN2(dq_des[1],dqm) * _y_ls[20] * t94 + t119 * _y_ls[12] * t94 + t150 * t94 - 0.2e1 * t150 * t94 * t18 - t155 * t94 + 0.2e1 * t155 * t94 * t53 - t111 * t33 - t110 * t134 * t115;
	tau2 = -(t137 + t163) / t94;
	tau3 = _y_ls[6] * t1 * D13 * t123 - _y_ls[5] * t1 * D13 * t115 + _y_ls[25] - _y_ls[6] * t99 * t40 - 0.2e1 * t105 * t106 - _y_ls[6] * g1 * t4 + _y_ls[5] * g1 * t8 + t155 - 0.2e1 * t155 * t53 + dq[2] * _y_ls[21] + SSIGN2(dq_des[2],dqm) * _y_ls[22] - _y_ls[5] * t99 * t43;

	tau4 = (_y_ls2[1*3+0]*dq[3] + _y_ls2[2*3+0]*SSIGN2(dq_des[3],dqm) + _y_ls2[3*3+0]);
	tau5 = (_y_ls2[1*3+1]*dq[4] + _y_ls2[2*3+1]*SSIGN2(dq_des[4],dqm) + _y_ls2[3*3+1]);
	tau6 = (_y_ls2[1*3+2]*dq[5] + _y_ls2[2*3+2]*SSIGN2(dq_des[5],dqm) + _y_ls2[3*3+2]);


// 	printf("tau1: %f \n",tau1);
// 	printf("tau2: %f \n",tau2);
// 	printf("tau3: %f \n",tau3);
// 	printf("tau4: %f \n",tau4);
// 	printf("tau5: %f \n",tau5);
// 	printf("tau6: %f \n",tau6);

	/* Mass matrix
	*/ 
	mt1 = cos(q[1]);
	mt2 = mt1 * mt1;
	mt3 = sin(q[1]);
	mt4 = mt3 * mt3;
	mt10 = sin(q[2]);
	mt11 = D13 * mt10;
	mt12 = q[1] + q[2];
	mt13 = sin(mt12);
	mt14 = mt13 * mt13;
	mt15 = cos(mt12);
	mt16 = mt15 * mt15;
	mt17 = mt14 - mt16;
	mt19 = cos(q[2]);
	mt20 = D13 * mt19;
	mt21 = mt13 * mt15;
	M[0+0*6] = _y_ls[0] + (mt2 - mt4) * _y_ls[1] - 0.2e1 * mt1 * mt3 * _y_ls[2] + (mt11 - mt11 * mt17 - 0.2e1 * mt20 * mt21) * _y_ls[5] + (mt20 - 0.2e1 * mt11 * mt21 + mt20 * mt17) * _y_ls[6] + mt17 * _y_ls[7] + 0.2e1 * mt21 * _y_ls[8];
	mt36 = mt13 * _y_ls[9];
	mt37 = mt15 * _y_ls[10];
	M[0+1*6] = mt1 * _y_ls[3] - mt3 * _y_ls[4] + mt36 + mt37;
	M[0+2*6] = mt36 + mt37;
	M[1+0*6] = M[0+1*6];
	mt38 = mt11 * _y_ls[5];
	mt40 = mt20 * _y_ls[6];
	M[1+1*6] = 0.2e1 * mt38 + 0.2e1 * mt40 + _y_ls[13] + _y_ls[14] + _y_ls[15] / 0.1000e4;
	M[1+2*6] = mt38 + mt40 + _y_ls[14] + r3 * _y_ls[15] / 0.1000e4;
	M[2+0*6] = M[0+2*6];
	M[2+1*6] = M[1+2*6];
	mt45 = r3 * r3;
	M[2+2*6] = _y_ls[14] + mt45 * _y_ls[15] / 0.1000e4;

	M[4+3*6] = 0.0;
	M[5+3*6] = 0.0;
	M[3+4*6] = 0.0;
	M[5+4*6] = 0.0;
	M[3+5*6] = 0.0;
	M[4+5*6] = 0.0;
	M[3+3*6] = _y_ls2[0*3+0];
	M[4+4*6] = _y_ls2[0*3+1];
	M[5+5*6] = _y_ls2[0*3+2];

	//Torque due to mass: M*ddq
	tauM[0] = M[0+0*6]*ddq[0]+M[0+1*6]*ddq[1]+M[0+2*6]*ddq[2];
	tauM[1] = M[1+0*6]*ddq[0]+M[1+1*6]*ddq[1]+M[1+2*6]*ddq[2];
	tauM[2] = M[2+0*6]*ddq[0]+M[2+1*6]*ddq[1]+M[2+2*6]*ddq[2];
	tauM[3] = M[3+3*6]*ddq[3];
	tauM[4] = M[4+4*6]*ddq[4];
	tauM[5] = M[5+5*6]*ddq[5];

	_tau[0] = torque_scale[0]* (tauM[0] + tau1) + torque_offset[0];
	_tau[1] = torque_scale[1]* (tauM[1] + tau2) + torque_offset[1];
	_tau[2] = torque_scale[2]* (tauM[2] + tau3) + torque_offset[2];
	_tau[3] = torque_scale[3]* (tauM[3] + tau4) + torque_offset[3];
	_tau[4] = torque_scale[4]* (tauM[4] + tau5) + torque_offset[4];
	_tau[5] = torque_scale[5]* (tauM[5] + tau6) + torque_offset[5];

	return _tau;
}
//}

