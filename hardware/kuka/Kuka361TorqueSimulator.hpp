
#ifndef KUKA361_TORQUE_SIMULATOR_HPP
#define KUKA361_TORQUE_SIMULATOR_HPP

#include <vector>
#include <typeinfo>
#include "kuka361FwDynnf.hpp"
#include <rtt/Logger.hpp>
#include <rtt/TimeService.hpp>
#include "dev/SimulationAxis.hpp"
#include "dev/TorqueSimulationAxis.hpp"

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
                OCL::TorqueSimulationAxis* sim_axis = dynamic_cast<OCL::TorqueSimulationAxis*>(_axes[axis]);
                if(sim_axis!=NULL){
                    if(!sim_axis->isDriven() || firststep[axis]) {
                        _acc_sim[axis] = _vel_sim[axis] = 0.0;
                        if (sim_axis->isDriven()) firststep[axis] = false;
                    }
                    ((TorqueSimulationVelocitySensor*) sim_axis->getSensor("Velocity"))->update(_vel_sim[axis], _acc_sim[axis], _previous_time);
                    ((TorqueSimulationEncoder*) sim_axis->getSensor("Position"))->update(_pos_sim[axis], _vel_sim[axis], _previous_time);
                    _vel_sim[axis] = sim_axis->getSensor("Velocity")->readSensor();
                    _pos_sim[axis] = sim_axis->getSensor("Position")->readSensor();
                    
                } else {
                    _vel_sim[axis] = _axes[axis]->getDriveValue();
                    _pos_sim[axis] = _axes[axis]->getSensor("Position")->readSensor();
                    _tau_sim[axis] = 0.0;
                }
            }
            _previous_time = TimeService::Instance()->getTicks();
            
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
