// Copyright (C) 2006-2008 Ruben Smits <ruben.smits@mech.kuleuven.be>
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

#ifndef __KUKA361NAXESACCELERATIONCONTROLLER__
#define __KUKA361NAXESACCELERATIONCONTROLLER__

#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Command.hpp>
#include <rtt/TimeService.hpp>

#include "kuka361InvDynnf.hpp"
#include "kuka361FwDynnf.hpp"

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that reads out the
     * output acceleration dataports OCl::nAxesController and
     * puts these output values in the driveValue dataports of an
     * nAxesTorqueController. 
     * 
     */

    class Kuka361nAxesAccelerationController : public RTT::TaskContext
    {
    public:
        /** 
         * The contructor of the class 
         * 
         * @param name name of the Taskcontext
         * 
         */
        Kuka361nAxesAccelerationController(const std::string& name);
        virtual ~Kuka361nAxesAccelerationController();
        
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        virtual void cleanupHook();
        
    private:
        bool startCalibration(double duration, double vmin, double vmax);
        bool finishedCalibration() const;
        void startMonitoring();
        void stopMonitoring();
        void Calculate();
        std::vector<double> Filter(vector<double>& x);
        
        unsigned int        num_axes;
        std::vector<double> q;
        std::vector<double> qdot;
        std::vector<double> qdot_des;
        std::vector<double> qddot_con;
        std::vector<double> driveValue;

        kuka361InvDynnf*	kuka361INVDYN;
        kuka361FwDynnf		kuka361FWDYN;
    protected:
        
        RTT::DataPort< std::vector<double> > q_meas_Port;
        RTT::DataPort< std::vector<double> > qdot_meas_Port, qdot_des_Port;
        RTT::DataPort< std::vector<double> > q_con_Port;
        RTT::WriteDataPort< bool > saturationPort;
        RTT::WriteDataPort<std::vector<double> > drives;
        RTT::ReadDataPort<std::vector<double> > torque_meas_Port;
        std::vector<double>torque_meas_local;
        
        /// Parameter for sign function of kuka361InvDynnf
        RTT::Property< double > dqm;
        RTT::Property< bool > UseDesVel;
        RTT::Property< std::vector<double> > torque_offset;
        RTT::Property< std::vector<double> > torque_scale;
        RTT::Property<double> Ts;
        
    private:
        bool saturated, ReportedSaturation;
        std::vector<double>     Km;
        unsigned int num_samples, samplenumber, initDataVecSize, dataVecSize;
        double time_sleep;
        RTT::TimeService::ticks time_begin;
        
        bool isCalibrating, isMonitoring;
        std::vector<double> time_data;
        std::vector< std::vector<double> > q_data, qdot_data, qddot_data, tau_meas_data, tau_model_data;
        RTT::Command<bool(double,double,double)>	_Calibrate;
    
  }; // class
}//namespace
#endif // __N_AXES_EFFECTOR_H__
