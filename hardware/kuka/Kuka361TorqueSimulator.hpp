
#ifndef KUKA361_TORQUE_SIMULATOR_HPP
#define KUKA361_TORQUE_SIMULATOR_HPP

#include <vector>
#include <typeinfo>
#include "kuka361dyn.hpp"
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
          _first_time(true)
        {};

        ~Kuka361TorqueSimulator(){};

         void update(vector<double> &_tau_sim) {
		if(_first_time){
			_first_time = false;
		} else {
			_delta_time = TimeService::Instance()->secondsSince(_previous_time);
			for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
				type = typeid(*_axes[axis]).name();
				if(type == "N3RTT20TorqueSimulationAxisE"){
					if(!_axes[axis]->isDriven()) _tau_sim[axis] = _acc_sim[axis] = _vel_sim[axis] = 0.0;
					_vel_sim[axis] += _acc_sim[axis]*(double)_delta_time;
					((TorqueSimulationVelocitySensor*) _axes[axis]->getSensor("Velocity"))->update(_vel_sim[axis], _acc_sim[axis], _previous_time);
					_pos_sim[axis] += _vel_sim[axis]*(double)_delta_time;
					((TorqueSimulationEncoder*) _axes[axis]->getSensor("Position"))->update(_pos_sim[axis], _vel_sim[axis], _previous_time);
				} 
				else {
					_vel_sim[axis] = _axes[axis]->getDriveValue();
					_pos_sim[axis] = _axes[axis]->getSensor("Position")->readSensor();
					_tau_sim[axis] = 0.0;
				}
			}
		}
		_previous_time = TimeService::Instance()->getTicks();
		_acc_sim = kuka361DM.fwdyn361( _tau_sim, _pos_sim, _vel_sim);
		//Logger::log()<<Logger::Debug<<"pos (rad): "<<_pos_sim[3]<<" | vel (rad/s): "<<_vel_sim[3]<<" | acc (rad/s2): "<<_acc_sim[3]<<" | tau (Nm): "<<_tau_sim[3]<<Logger::endl;
         };
        
    private:
        vector<RTT::AxisInterface*> _axes;
        vector<double> _pos_sim;
        vector<double> _vel_sim;
        vector<double> _acc_sim;
	TimeService::Seconds _delta_time;
	TimeService::ticks _previous_time;
	bool _first_time;
	string type;
	kuka361dyn kuka361DM;
    }; // class
}//namespace
#endif // Kuka361TorqueSimulator

#ifndef KUKA361_TORQUE_SIMULATOR_HPP
#define KUKA361_TORQUE_SIMULATOR_HPP

#include <vector>
#include <typeinfo>
#include "kuka361dyn.hpp"
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
          _first_time(true)
        {};

        ~Kuka361TorqueSimulator(){};

         void update(vector<double> &_tau_sim) {
		if(_first_time){
			_first_time = false;
		} else {
			_delta_time = TimeService::Instance()->secondsSince(_previous_time);
			for (int axis=0;axis<KUKA361_NUM_AXES;axis++) {
				type = typeid(*_axes[axis]).name();
				if(type == "N3RTT20TorqueSimulationAxisE"){
					if(!_axes[axis]->isDriven()) _tau_sim[axis] = _acc_sim[axis] = _vel_sim[axis] = 0.0;
					_vel_sim[axis] += _acc_sim[axis]*(double)_delta_time;
					((TorqueSimulationVelocitySensor*) _axes[axis]->getSensor("Velocity"))->update(_vel_sim[axis], _acc_sim[axis], _previous_time);
					_pos_sim[axis] += _vel_sim[axis]*(double)_delta_time;
					((TorqueSimulationEncoder*) _axes[axis]->getSensor("Position"))->update(_pos_sim[axis], _vel_sim[axis], _previous_time);
				} 
				else {
					_vel_sim[axis] = _axes[axis]->getDriveValue();
					_pos_sim[axis] = _axes[axis]->getSensor("Position")->readSensor();
					_tau_sim[axis] = 0.0;
				}
			}
		}
		_previous_time = TimeService::Instance()->getTicks();
		_acc_sim = kuka361DM.fwdyn361( _tau_sim, _pos_sim, _vel_sim);
		//Logger::log()<<Logger::Debug<<"pos (rad): "<<_pos_sim[3]<<" | vel (rad/s): "<<_vel_sim[3]<<" | acc (rad/s2): "<<_acc_sim[3]<<" | tau (Nm): "<<_tau_sim[3]<<Logger::endl;
         };
        
    private:
        vector<RTT::AxisInterface*> _axes;
        vector<double> _pos_sim;
        vector<double> _vel_sim;
        vector<double> _acc_sim;
	TimeService::Seconds _delta_time;
	TimeService::ticks _previous_time;
	bool _first_time;
	string type;
	kuka361dyn kuka361DM;
    }; // class
}//namespace
#endif // Kuka361TorqueSimulator
