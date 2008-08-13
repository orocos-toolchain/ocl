/**************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef _TORQUE_SIM_AXIS_HPP
#define _TORQUE_SIM_AXIS_HPP

#include <ocl/OCL.hpp>
#include <rtt/dev/AxisInterface.hpp>
#include <rtt/dev/DigitalOutput.hpp>
#include <rtt/TimeService.hpp>

namespace OCL
{
    /** @brief Helper class that implements a Position Sensor for a
	TorqueSimulationAxis
	@see TorqueSimulationAxis
    */
    class TorqueSimulationEncoder: public SensorInterface<double>
    {
    public:
	TorqueSimulationEncoder(double initial=0, double min=-10, double max=10);
	virtual ~TorqueSimulationEncoder() {};

	virtual double readSensor() const;
	virtual int readSensor(double& data) const;
	virtual double maxMeasurement() const {return _max; };
	virtual double minMeasurement() const {return _min; };
	virtual double zeroMeasurement() const {return 0.0;};

	void update(double position, double velocity, TimeService::ticks previous_time);
	void stop();

    private:
	double _position, _velocity, _min, _max;
	TimeService::ticks _previous_time;
	TimeService::Seconds _delta_time;
	bool _first_drive;

    }; // class

     /** @brief Helper class that implements a Velocity Sensor for a
	TorqueSimulationAxis
    */
    class TorqueSimulationVelocitySensor : public SensorInterface<double>
    {
    public:
	TorqueSimulationVelocitySensor(double maxvel);

	virtual ~TorqueSimulationVelocitySensor() {} //std::cout << "TorqueSimulationVelocitySensor destructor" << std::endl; }

	virtual int readSensor( double& vel ) const;
	virtual double readSensor() const;
	virtual double maxMeasurement() const { return _maxvel; }
	virtual double minMeasurement() const { return - _maxvel; }
	virtual double zeroMeasurement() const { return 0; }

	void update(double velocity, double acceleration, TimeService::ticks previous_time);
	void stop();

    private:
	double _velocity, _acceleration, _maxvel;
	TimeService::ticks _previous_time;
	TimeService::Seconds _delta_time;
	bool _first_drive;

    }; // class


    // Forward declare; see below
	class TorqueSimulationCurrentSensor;

    /** @brief Simple implementation of a non-physical axis for
	simulation of a machine/robot
     */
    class TorqueSimulationAxis: public AxisInterface
    {
    public:
	TorqueSimulationAxis(double initial=0, double min=-10, double max=10, double velLim=2);
	virtual ~TorqueSimulationAxis();

	virtual bool stop();
	virtual bool lock();
	virtual bool unlock();
        virtual bool drive( double cur );
	virtual bool isLocked() const;
	virtual bool isStopped() const;
	virtual bool isDriven() const;
	virtual SensorInterface<double>* getSensor(const std::string& name) const;
	virtual std::vector<std::string> sensorList() const;
	virtual SensorInterface<int>* getCounter(const std::string& name) const { return  0;}
	virtual std::vector<std::string> counterList() const { return std::vector<std::string>();}
	virtual DigitalInput* getSwitch(const std::string& name) const { return 0; }
	virtual std::vector<std::string> switchList() const { return std::vector<std::string>();}

	double getDriveValue() const;
	void   setMaxDriveValue( double cur_max ) { _max_drive_value = cur_max; }

    DigitalOutput* getBrake();
    DigitalOutput* getEnable();

    private:
	double      _drive_value;
	DigitalOutput  _enable;
	DigitalOutput  _brake;
	double      _max_drive_value;
	TorqueSimulationEncoder*         _encoder;
	TorqueSimulationVelocitySensor*  _velSensor;
	TorqueSimulationCurrentSensor*  _curSensor;
	bool _is_locked, _is_stopped, _is_driven;


    }; // class


    /** @brief Helper class that implements a Current Sensor for a
	TorqueSimulationAxis
    */
    class TorqueSimulationCurrentSensor : public SensorInterface<double>
    {
    public:
	TorqueSimulationCurrentSensor(TorqueSimulationAxis* axis, double maxcur) : _axis(axis), _maxcur(maxcur)
	{};

	virtual ~TorqueSimulationCurrentSensor() {} //std::cout << "TorqueSimulationCurrentSensor destructor" << std::endl; }

	virtual int readSensor( double& cur ) const { cur = _axis->getDriveValue(); return 0; }

	virtual double readSensor() const { return _axis->getDriveValue(); }

	virtual double maxMeasurement() const { return _maxcur; }

	virtual double minMeasurement() const { return - _maxcur; }

	virtual double zeroMeasurement() const { return 0; }

    private:
	TorqueSimulationAxis* _axis;
	double _maxcur;
    };

} // namespace

#endif //_TORQUE_SIM_AXIS_HPP



