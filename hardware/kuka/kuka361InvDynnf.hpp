
#include <vector>
#include "math.h"
//namespace Orocos
//{
	using namespace std;


	/**
	*This class can calculate the inverse dynamics of the kuka 361
	*/
	class kuka361InvDynnf {
	
	
	public:
		kuka361InvDynnf(double dqm_);
		~kuka361InvDynnf(){};
	
		/** 
		* Calculate inverse dynamics of kuka 361
		*
		* 
		* @param q joint positions
		* @param dq joint velocities
		* @param ddq motor torques
		* 
		* @return joint torques
		*/
		vector<double> invdyn361(vector<double> &q, vector<double> &dq, vector<double> &dq_des, vector<double> &ddq, vector<double> &torque_scale, vector<double> &torque_offset);

	private:
		//Joint acceleration calculated by fwdyn361()
		vector<double> _tau;

		//Parameters of the kuka 361 forward dynamic model
		double dqm, D13, g1, r3, l, r;
		double _y_ls[26];
		double _y_ls2[12];
	
		//doubles necessary to calculate the matrix multiplication in an efficient way in fwdyn361() 
		double t1, t3, t4, t6, t8;
		double t12, t16, t17, t18, t30, t31, t32, t33, t36, t37, t38, t39, t40, t43, t49;
		double t52, t53, t57, t71, t87, tau1, t88, t89, t90, t94, t99;
		double t105, t106, t110, t111, t114, t115, t119, t122, t123, t126, t127, t134, t137, t150, t155;
		double t163, tau2, tau3, tau4, tau5, tau6;
		double mt1, mt2, mt3, mt4, mt10, mt11, mt12, mt13, mt14, mt15, mt16, mt17, mt19, mt20, mt21, mt36, mt37, mt38, mt40, mt45;
		double tauM[6];
		double M[36];
	};
//}//namespace Orocos
