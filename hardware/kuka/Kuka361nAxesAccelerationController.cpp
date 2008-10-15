// Copyright (C) 2008 Ruben Smits <ruben.smits@mech.kuleuven.be>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#include "Kuka361nAxesAccelerationController.hpp"
#include <rtt/Logger.hpp>
#include <assert.h>
#include <ocl/ComponentLoader.hpp>

ORO_LIST_COMPONENT_TYPE( OCL::Kuka361nAxesAccelerationController )

#define KUKA361_KM { 0.2781*94.14706, 0.2863*103.23529, 0.2887*51.44118, 0.07*175, 0.07*150, 0.07*131.64395 } 

#include "math.h"


namespace OCL
{
    using namespace RTT;
    using namespace std;
    
    Kuka361nAxesAccelerationController::Kuka361nAxesAccelerationController(const string& name)
        : TaskContext(name,PreOperational),
          num_axes(6),
          q(num_axes),
          qdot(num_axes),
          qdot_des(num_axes),
          qddot_con(num_axes),
          driveValue(num_axes),
          q_meas_Port("nAxesSensorPosition"),
          qdot_meas_Port("nAxesSensorVelocity"),
          qdot_des_Port("nAxesDesiredVelocity"),
          q_con_Port("nAxesOutputAcceleration"),
          saturationPort("saturationPort",false),
          drives("nAxesOutputTorque"),
          torque_meas_Port("nAxesSensorTorque"),
          torque_meas_local(num_axes),
          dqm("dqm", "SSIGN parameter for kuka361InvDynnf"),
          UseDesVel("UseDesVel", "Use desired velocity for the coulomb friction term in Dynamic model"),
          torque_offset("torque_offset", "correction for torque offset in dynamic model"),
          torque_scale("torque_scale", "correction for torque scale in dynamic model"),
          Ts("Ts","Timestep to use",0.01),
          saturated(false),
          ReportedSaturation(false),
          Km(6),
          time_data(1),
          q_data(6),
          qdot_data(6),
          qddot_data(6),
          tau_meas_data(6),
          tau_model_data(6)
    {
        //Creating TaskContext
        
        //Adding ports
        this->ports()->addPort(&drives);
        this->ports()->addPort(&torque_meas_Port);
        this->ports()->addPort(&q_meas_Port);
        this->ports()->addPort(&qdot_meas_Port);
        this->ports()->addPort(&qdot_des_Port);
        this->ports()->addPort(&q_con_Port);
        this->ports()->addPort(&saturationPort);
        
        //Adding Properties
        this->properties()->addProperty(&dqm);
        this->properties()->addProperty(&UseDesVel);
        this->properties()->addProperty(&torque_scale);
        this->properties()->addProperty(&torque_offset);
        this->properties()->addProperty(&Ts);
        
        //Adding Commands
        this->commands()->addCommand(command( "Calibrate_move", &Kuka361nAxesAccelerationController::startCalibration,
                                              &Kuka361nAxesAccelerationController::finishedCalibration, this),
                                     "Calibrate scale and offset for dynamic model",
                                     "duration", "duration of calibration",
                                     "vmin", "minimum calibration velocity",
                                     "vmax", "maximum calibration velocity");

        //Adding Methods	
        this->methods()->addMethod( method( "Calibrate_calc", &Kuka361nAxesAccelerationController::Calculate, this),
                                    "Calculate model offsets and scales");
        this->methods()->addMethod( method( "Start_Monitoring", &Kuka361nAxesAccelerationController::startMonitoring, this),
                                    "start monitoring robot for trajectory optimization");
        this->methods()->addMethod( method( "Stop_Monitoring", &Kuka361nAxesAccelerationController::stopMonitoring, this),
                                    "stop monitoring robot for trajectory optimization");
        
        double KM[6] = KUKA361_KM;
        for(unsigned int i = 0;i<6;i++){
            Km[i] = KM[i];
        }
        
// 		//BEGIN Debug : Perfect acceleration
// 		_position_local[0] = 0*3.1416/180; 
// 		_position_local[1] = 27*3.1416/180; 
// 		_position_local[2] = 107*3.1416/180; 
// 		_position_local[3] = -110*3.1416/180; 
// 		_position_local[4] = 45*3.1416/180; 
// 		_position_local[5] = 110*3.1416/180;
// 		_position.Set(_position_local); 
// 		_velocity.Set(_velocity_local); 
// 		//END Debug
    }
    
    bool Kuka361nAxesAccelerationController::configureHook()
    {
        kuka361INVDYN = new kuka361InvDynnf(dqm.value());
        return true;
    }
    
    void Kuka361nAxesAccelerationController::cleanupHook()
    {
        delete kuka361INVDYN;
    }
    
    
    Kuka361nAxesAccelerationController::~Kuka361nAxesAccelerationController(){};
    
    bool Kuka361nAxesAccelerationController::startHook()
    {
        //Check if readPort is connected
        if (!q_meas_Port.connected())
            Logger::log()<<Logger::Warning<<"(Kuka361nAxesAccelerationController) Port "<<q_meas_Port.getName()<<" not connected"<<Logger::endl;
        if (!qdot_meas_Port.connected())
            Logger::log()<<Logger::Warning<<"(Kuka361nAxesAccelerationController) Port "<<qdot_meas_Port.getName()<<" not connected"<<Logger::endl;
        if (!qdot_des_Port.connected())
            Logger::log()<<Logger::Warning<<"(Kuka361nAxesAccelerationController) Port "<<qdot_des_Port.getName()<<" not connected"<<Logger::endl;
        if (!q_con_Port.connected())
            Logger::log()<<Logger::Warning<<"(Kuka361nAxesAccelerationController) Port "<<q_con_Port.getName()<<" not connected"<<Logger::endl;
        
        //Initialize
        isCalibrating = false;
        isMonitoring = false;
        saturated = false;
        ReportedSaturation = false;
        return true;
    }
    
    void Kuka361nAxesAccelerationController::updateHook()
    {
        // Read Measurements
        q = q_meas_Port.Get();
        qdot = qdot_meas_Port.Get();
        torque_meas_local = torque_meas_Port.Get();
        
        qddot_con = q_con_Port.Get();//Desired Accelerations

        // Read desired velocities
        qdot_des = qdot_des_Port.Get();
        
        //Use inverse dynamic model to calculate torques from accelerations
        if(UseDesVel.value()){
            driveValue=kuka361INVDYN->invdyn361(q,qdot,qdot_des,qddot_con,torque_scale.value(),torque_offset.value());
        }else{
            driveValue=kuka361INVDYN->invdyn361(q,qdot,qdot,qddot_con,torque_scale.value(),torque_offset.value());
        }
        
        //saturate torques at 11*Km
        for (unsigned int i=0;i<num_axes;i++) {
            if(driveValue[i] < -11.0*Km[i]) {
                driveValue[i] = -11.0*Km[i];
                saturated = true;
                saturationPort.Set(true);
            }
            else if(driveValue[i] > 11.0*Km[i]) {
                driveValue[i] = 11.0*Km[i];
                saturated = true;
                saturationPort.Set(true);
            }
            else	saturationPort.Set(false);
        }
        
        
        if(saturated && !ReportedSaturation) {
            log(Warning)<<"Drive Has been saturated!"<<endlog();
            ReportedSaturation=true;
        }
        
        
        if (isCalibrating && TimeService::Instance()->secondsSince(time_begin) > time_sleep)
            {
                for(unsigned int axis=0;axis<num_axes;axis++){
                    q_data[axis][samplenumber]=q[axis];
                    tau_meas_data[axis][samplenumber] = torque_meas_local[axis];
                }
                time_data[samplenumber] = TimeService::Instance()->secondsSince(time_begin);
                for(unsigned int axis=0;axis<num_axes;axis++){
                    qdot_data[axis][samplenumber]=qdot[axis];
                }
                
                samplenumber++;
                if (samplenumber == num_samples)	isCalibrating = false;
            }
        
        if (isMonitoring && TimeService::Instance()->secondsSince(time_begin) > time_sleep)
            {
                for(unsigned int axis=0;axis<num_axes;axis++){
                    q_data[axis][samplenumber]=q[axis];
                    tau_meas_data[axis][samplenumber] = torque_meas_local[axis];
                }
                time_data[samplenumber] = TimeService::Instance()->secondsSince(time_begin);
                for(unsigned int axis=0;axis<num_axes;axis++){
                    qdot_data[axis][samplenumber]=qdot[axis];
                }
                
                samplenumber++;
                if (samplenumber >= time_data.size()) {
                    for(unsigned int axis=0;axis<num_axes;axis++){
                        q_data[axis].resize(2*q_data[axis].size());
                        tau_meas_data[axis].resize(2*tau_meas_data[axis].size());
                        qdot_data[axis].resize(2*qdot_data[axis].size());
                    }
                    time_data.resize(2*time_data.size());
                }
            }
        
        
        //write torque values to output ports
        drives.Set(driveValue);
        
// 		//BEGIN Debug Perfect Acceleration
// 			double Ts = 0.01;
// 			_acceleration_out_local = _kuka361FWDYN.fwdyn361(driveValue, _position_local, _velocity_local);
// 			for(unsigned int i=0; i<6; i++) {
// 				_velocity_local[i] = _velocity_local[i] + _acceleration_out_local[i]*Ts;
// 				_position_local[i] = _position_local[i] + _velocity_local[i]*Ts;
// 			}
// 			_position.Set(_position_local);
// 			_velocity.Set(_velocity_local);
// 
// 		log(Debug)<<" "<<endlog();
// 		log(Debug)<<"_acceleration_out_local "<<_acceleration_out_local[0]<<" "<<_acceleration_out_local[1]<<" "<<_acceleration_out_local[2]<<" "<<_acceleration_out_local[3]<<" "<<_acceleration_out_local[4]<<" "<<_acceleration_out_local[5]<<endlog();
// 		//END DEBUG

// 	log(Debug)<<"Updating Effector done"<<endlog();
    }
    
    void Kuka361nAxesAccelerationController::stopHook()
    {
        for (unsigned int i=0; i<num_axes; i++){
            drives.Set(vector<double>(num_axes,0.0));
        }
    }
    
    
    bool Kuka361nAxesAccelerationController::startCalibration(double duration, double vmin, double vmax)
    {
        log(Warning)<<"(Kuka361nAxesAccelerationController) start ModelCalibration"<<endlog();
        
        // don't do anything if still calibrating
        if (isCalibrating)
            return false;
        
        // get new measurement
        else{
            
            _Calibrate = getPeer("nAxesGeneratorSin")->commands()->getCommand<bool(double,double,double)>("Calibrate");
            assert( _Calibrate.ready() );
            _Calibrate(duration, vmin, vmax);
// 		if (!result) log(Error)<<"Swept Sine could not be started"<<endlog();
            
            time_sleep        = 1.;//avoid first and last 1 seconds
            time_begin        = TimeService::Instance()->getTicks();
            num_samples       = (int)(duration/Ts.rvalue() - 2*time_sleep/Ts.rvalue()); //avoid first and last data of measurement
            
            time_data.resize(num_samples);
            for(unsigned int axis=0;axis<num_axes;axis++){
                q_data[axis].resize(num_samples);
                qdot_data[axis].resize(num_samples);
                qddot_data[axis].resize(num_samples-1);
                tau_meas_data[axis].resize(num_samples);
                tau_model_data[axis].resize(num_samples-1);
            }
            
            samplenumber = 0;
            isCalibrating = true;
            return true;
        }
    }

    bool Kuka361nAxesAccelerationController::finishedCalibration() const
    {
        return !isCalibrating;
    }
    
    void Kuka361nAxesAccelerationController::startMonitoring()
    {
        if (!isMonitoring){
            log(Warning)<<"(Kuka361nAxesAccelerationController) start Monitoring"<<endlog();
            
            time_sleep        = 0.1;//avoid first time_sleep seconds
            time_begin        = TimeService::Instance()->getTicks();
            initDataVecSize   = (int)(20/Ts.rvalue());
            
            time_data.resize(initDataVecSize,0);
            for(unsigned int axis=0;axis<num_axes;axis++){
                q_data[axis].resize(initDataVecSize);
                qdot_data[axis].resize(initDataVecSize);
                tau_meas_data[axis].resize(initDataVecSize);
            }
            
            samplenumber = 0;
            isMonitoring = true;
            getPeer("Reporting")->start();
        }
    }
    
    void Kuka361nAxesAccelerationController::stopMonitoring()
    {
        log(Warning)<<"(Kuka361nAxesAccelerationController) stop Monitoring"<<endlog();
        
        isMonitoring = false;
        getPeer("Reporting")->stop();
        
        unsigned int sample=0;
        while (time_data[sample] != 0 ){
            sample++;
        }
        dataVecSize = sample-50; //Ignore last 50 samples
        num_samples = dataVecSize;
        
        time_data.resize(dataVecSize);
        for(unsigned int axis=0;axis<num_axes;axis++){
            q_data[axis].resize(dataVecSize);
            qdot_data[axis].resize(dataVecSize);
            tau_meas_data[axis].resize(dataVecSize);
            qddot_data[axis].resize(dataVecSize-1);
            tau_model_data[axis].resize(dataVecSize-1);
        }
    }
    
    
    void Kuka361nAxesAccelerationController::Calculate(){
        
        log(Warning)<<"calculation Started"<<endlog();
        
        //calculate qddot from qdot_data and time_data
        for(unsigned int axis=0;axis<num_axes;axis++){
            for(unsigned int sample=0;sample<num_samples-1;sample++){
                qddot_data[axis][samplenumber] = (qdot_data[axis][sample+1] - qdot_data[axis][sample])/(time_data[sample+1]-time_data[sample]);
            }
        }
        
        //low-pass filter to filter out most noise
        for(unsigned int axis=0;axis<num_axes;axis++){
            qdot_data[axis] = Filter(qdot_data[axis]);
            qddot_data[axis] = Filter(qddot_data[axis]);
            tau_meas_data[axis] = Filter(tau_meas_data[axis]);
        }
        
        //Calculate torque according to model
        vector<double> q_sample(6);
        vector<double> qdot_sample(6);
        vector<double> qddot_sample(6);
        vector<double> tau_model_sample(6);
        vector<double> q_zero(6,0);
        vector<double> q_ones(6,1);
        
        for(unsigned int sample=0;sample<num_samples-1;sample++){
            for(unsigned int axis=0;axis<num_axes;axis++){
                q_sample[axis] = q_data[axis][sample];
                qdot_sample[axis] = qdot_data[axis][sample];
                qddot_sample[axis] = qddot_data[axis][sample];
            }
            tau_model_sample = kuka361INVDYN->invdyn361(q_sample,qdot_sample,qdot_sample,qddot_sample,q_ones,q_zero);
            for(unsigned int axis=0;axis<num_axes;axis++){
                tau_model_data[axis][sample] = tau_model_sample[axis];
            }
        }
        
        log(Debug)<<"tau_meas_data 1: "<<tau_meas_data[0][1000]<<" "<<tau_meas_data[0][5000]<<" "<<tau_meas_data[0][8000]<<endlog();
        log(Debug)<<"tau_model_data 1: "<<tau_model_data[0][1000]<<" "<<tau_model_data[0][5000]<<" "<<tau_model_data[0][8000]<<endlog();
        log(Debug)<<"tau_meas_data 3: "<<tau_meas_data[2][1000]<<" "<<tau_meas_data[2][5000]<<" "<<tau_meas_data[2][8000]<<endlog();
        log(Debug)<<"tau_model_data 3: "<<tau_model_data[2][1000]<<" "<<tau_model_data[2][5000]<<" "<<tau_model_data[2][8000]<<endlog();
        log(Debug)<<"tau_meas_data 4: "<<tau_meas_data[3][1000]<<" "<<tau_meas_data[3][5000]<<" "<<tau_meas_data[3][8000]<<endlog();
        log(Debug)<<"tau_model_data 4: "<<tau_model_data[3][1000]<<" "<<tau_model_data[3][5000]<<" "<<tau_model_data[3][8000]<<endlog();

        vector<double> offsets(6,0);
        for (unsigned int axis=0; axis<num_axes; axis++){
            for (unsigned int sample=0; sample<num_samples-1; sample++){
                offsets[axis] += (tau_meas_data[axis][sample] - tau_model_data[axis][sample])/(num_samples-1);
            }
        }
        vector<double> scales(6,0);
        for (unsigned int axis=3; axis<num_axes; axis++){
            for (unsigned int sample=0; sample<num_samples-1; sample++){
                scales[axis] += (tau_meas_data[axis][sample] / tau_model_data[axis][sample])/(num_samples-1);
            }
        }
        scales[0]=scales[1]=scales[2]=1;
        
        torque_offset.value() = offsets;
        torque_scale.value() = scales;
        
        log(Warning)<<"calculation Finished"<<endlog();
    }
    
    vector<double> Kuka361nAxesAccelerationController::Filter(vector<double>& x){
        vector<double> a(6);
        a[0] = 1.;
        a[1] = -4.1873000478644;
        a[2] = 7.069722752792469;
        a[3] = -6.009958148187328;
        a[4] = 2.5704293025241;
        a[5] = -0.44220918239962;
        
        vector<double> b(6);
        b[0] = 0.000021396152038;
        b[1] = 0.000106980760191;
        b[2] = 0.000213961520382;
        b[3] = 0.000213961520382;
        b[4] = 0.000106980760191;
        b[5] = 0.000021396152038;
        
        vector<double> y(x.size());
        y[0] = b[0]*x[0];
        y[1] = b[0]*x[1] + b[1]*x[0] - a[1]*y[0];
        y[2] = b[0]*x[2] + b[1]*x[1] + b[2]*x[0] - a[1]*y[1] - a[2]*y[0];
        y[3] = b[0]*x[3] + b[1]*x[2] + b[2]*x[1] + b[3]*x[0] - a[1]*y[2] - a[2]*y[1] - a[3]*y[0];
        y[4] = b[0]*x[4] + b[1]*x[3] + b[2]*x[2] + b[3]*x[1] + b[4]*x[0] - a[1]*y[3] - a[2]*y[2] - a[3]*y[1] - a[4]*y[0];
        
        for (unsigned int i=5; i<x.size(); i++){
            y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2] + b[3]*x[i-3] + b[4]*x[i-4] + b[5]*x[i-5] - a[1]*y[i-1] - a[2]*y[i-2] - a[3]*y[i-3] - a[4]*y[i-4] - a[5]*y[i-5];
        }
        
        return y;
    }
    
}//namespace




