
#ifndef KUKA361_TORQUE_SIMULATOR_HPP
#define KUKA361_TORQUE_SIMULATOR_HPP

#include <vector>
#include <typeinfo>
#include "kuka361FwDynnf.hpp"
#include <rtt/Logger.hpp>

#include <ocl/OCL.hpp>

#define KUKA361_NUM_AXES 6

namespace OCL
{
    /**
     * This class simulates a torque controlled kuka361
     * using its dynamic model and updates the position
     * and velocity sensor in the axis objects.
     *
     */

    class Kuka361TorqueSimulator
    {
    public:
        Kuka361TorqueSimulator(vector<RTT::AxisInterface*> &axes, vector<double> &initialPosition) :
          _axes(axes),
          _pos_sim(initialPosition),
          _vel_sim(KUKA361_NUM_AXES, 0.0),
          _acc_sim(KUKA361_NUM_AXES, 0.0),
          firststep(6,true)
        {};

        ~Kuka361TorqueSimulator(){};

         void update(vector<double> &_tau_sim) {
		_acc_sim = kuka361FWDYN.fwdyn361( _tau_sim, _pos_sim, _vel_sim);
		for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
			type = typeid(*_axes[axis]).name();
			if(type == "N3RTT20TorqueSimulationAxisE"){
				if(!_axes[axis]->isDriven() || firststep[axis]) {
					_acc_sim[axis] = _vel_sim[axis] = 0.0;
					if (_axes[axis]->isDriven()) firststep[axis] = false;
				}
				((TorqueSimulationVelocitySensor*) _axes[axis]->getSensor("Velocity"))->update(_vel_sim[axis], _acc_sim[axis], _previous_time);
				((TorqueSimulationEncoder*) _axes[axis]->getSensor("Position"))->update(_pos_sim[axis], _vel_sim[axis], _previous_time);
				_vel_sim[axis] = _axes[axis]->getSensor("Velocity")->readSensor();
				_pos_sim[axis] = _axes[axis]->getSensor("Position")->readSensor();

			} else {
				_vel_sim[axis] = _axes[axis]->getDriveValue();
				_pos_sim[axis] = _axes[axis]->getSensor("Position")->readSensor();
				_tau_sim[axis] = 0.0;
			}
		}
		_previous_time = TimeService::Instance()->getTicks();

// 		log(Warning) <<"_tau_sim: "<<_tau_sim[1]<<endlog();
// 	        log(Warning) <<"_pos_sim: "<<_pos_sim[1]<<endlog();
// 	        log(Warning) <<"_vel_sim: "<<_vel_sim[1]<<endlog();
// 	        log(Warning) <<"_acc_sim: "<<_acc_sim[1]<<endlog();
// 	        log(Warning) <<"_tau_sim[0]: "<<_tau_sim[0]<<endlog();
		//Logger::log()<<Logger::Debug<<"pos (rad): "<<_pos_sim[3]<<" | vel (rad/s): "<<_vel_sim[3]<<" | acc (rad/s2): "<<_acc_sim[3]<<" | tau (Nm): "<<_tau_sim[3]<<Logger::endl;
         };

    private:
        vector<RTT::AxisInterface*> _axes;
        vector<double> _pos_sim;
        vector<double> _vel_sim;
        vector<double> _acc_sim;
	TimeService::ticks _previous_time;
	string type;
	vector<bool> firststep;
	kuka361FwDynnf kuka361FWDYN;
    }; // class
}//namespace
#endif // Kuka361TorqueSimulator
