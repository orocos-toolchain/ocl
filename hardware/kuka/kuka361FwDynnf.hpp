#ifndef __kuka361FwDynnf_H__
#define __kuka361FwDynnf_H__


#include <vector>
#include "math.h"
//namespace Orocos
//{
	using namespace std;


	/**
	*This class can calculate the forward dynamics of the kuka 361
	*/
	class kuka361FwDynnf {
	
	
	public:
		kuka361FwDynnf();
		~kuka361FwDynnf(){};
	
		/** 
		* Calculate forward dynamics of kuka 361
		*
		* 
		* @param tau motor torques
		* @param q joint positions
		* @param dq joint velocities
		* 
		* @return joint acceleration
		*/
		vector<double> fwdyn361(vector<double> &tau, vector<double> &q, vector<double> &dq);

	private:
		//Joint acceleration calculated by fwdyn361()
		vector<double> _ddq;

		//Parameters of the kuka 361 forward dynamic model
		double dqm, D13, g1, r3, l, r;
		double _y_ls[26];
		double _y_ls2[12];
	
		//doubles necessary to calculate the matrix multiplication in an efficient way in fwdyn361() 
		double t1, t2, t3, t5, t6, t8, t9, t11;
		double t12, t15, t16, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31, t33, t34, t36, t37, t38, t39, t40, t42, t43, t46, t48, t49;
		double t50, t51, t52, t53, t54, t55, t59, t60, t61, t62, t63, t64, t65, t68, t69, t70, t71, t74, t75, t77, t80, t81, t82, t83, t86, t89, t90;
		double t92, t93, t94, t96, t97, t98, t101, t102, t103, t106, t108, t111, t113, t116, t121, t123, t124, t127, t128, t131, t133, t137, t140, t143, t144;
		double t145, t148, t149, t152, t155, t157, t158, t159, t164, t167, t169, t170, t173, t174, t175, t178, t179, t182, t186, t187, t190, t195, t200, t204, t207, t208;
		double t213, t217, t221, t225, t237, t240, t247, t252, t258, t268, t273, t276, t279, t284, t285, t292, t293, t296, t298, t310, t314, t321, t324, t329;
		double t331, t332, t335, t336, t337, t350, t351, t353, t354, t357, t358, t361, t366, t370, t371, t372, t378, t379, t384, t391, t398, t404, t414, t433;
		double t436, t460, t465, t476, t479, t489, t492, t511, t513, t514, t520, t530, t532, t533, t534, t540, t543, t546, t549, t553, t588, t592, t598, t627;
		double t638, t643, t647, t657, t662, t664, t667, t668, t669, t670, t673, t674, t680, t685, t693, t700, t706, t707, t730, t731, t732, t735, t743, t744, t748;
		double t750, t754, t756, t757, t759, t761, t762, t763, t764, t765, t766, t767, t768, t772, t775, t776, t781, t794, t795, t799, t800, t804, t808, t811;
		double t814, t817, t818, t838, t842, t858, t859, t878, t881, t887, t890, t893, t895, t899, t900, t904, t906, t907, t909, t911, t912, t917, t918, t921;
		double t927, t929, t930, t933, t940, t942, t944, t949, t953, t957, t959, t962, t964, t967, t970, t972, t974, t975, t977, t979, t980, t982, t984, t988;
		double t990, t991, t993, t996, t998, t1000, t1004, t1009, t1017, t1027, t1029, t1031, t1033, t1035, t1039, t1041, t1042, t1045, t1059, t1074, t1100, t1109;
		double t1138, t1143;
		double tau4, tau5, tau6;
		double taut[3];
	};
//}//namespace Orocos
#endif
