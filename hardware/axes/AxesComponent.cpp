/***************************************************************************
  tag: Peter Soetens  Thu Jul 15 11:21:06 CEST 2004  AxesComponent.cxx 

                        AxesComponent.cxx -  description
                           -------------------
    begin                : Thu July 15 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens at mech.kuleuven.ac.be
 
 ***************************************************************************
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

#include "AxesComponent.hpp"
#include <rtt/Method.hpp>
#include <rtt/Command.hpp>

namespace OCL 
{

    AxesComponent::AxesComponent( int max_chan , const std::string& name ) 
        :  TaskContext(name),
           max_channels("MaximumChannels","The maximum number of virtual sensor/drive channels", max_chan),
           chan_sensor("OutputValues",ChannelType(max_chan, 0.0) ),
           chan_drive("InputValues"),
           testData("testData", ChannelType(max_chan, 0.0) ),
           usingChannels(0)
    {
        this->attributes()->addAttribute(&testData);

        chan_out.resize(max_chan);
        chan_meas.resize(max_chan);

        this->methods()->addMethod( method( "switchOn", &AxesComponent::switchOn, this),
                           "Switch A Digital Output on",
                           "Name","The Name of the DigitalOutput."
                            ); 
        this->methods()->addMethod( method( "switchOff", &AxesComponent::switchOff, this),
                           "Switch A Digital Output off",
                           "Name","The Name of the DigitalOutput."
                            ); 
        this->methods()->addMethod( method( "enableAxis", &AxesComponent::enableAxis, this),
                           "Enable an Axis ( equivalent to unlockAxis( )",
                           "Name","The Name of the Axis."
                           ); 
        this->methods()->addMethod( method( "unlockAxis", &AxesComponent::enableAxis, this),
                           "Unlock an Axis. Enters 'stopped' state.",
                           "Name","The Name of the Axis."
                            ); 
        this->methods()->addMethod( method( "stopAxis", &AxesComponent::stopAxis, this),
                           "Stop an Axis from driven. Enters 'stopped' state.",
                           "Name","The Name of the Axis."
                            ); 
        this->methods()->addMethod( method( "lockAxis", &AxesComponent::disableAxis, this),
                           "Disable (lock an Axis. Enters 'locked' state/",
                           "Name","The Name of the Axis."
                           ); 
        this->methods()->addMethod( method( "disableAxis", &AxesComponent::disableAxis, this),
                           "Disable (lock an Axis (equivalent to lockAxis() )",
                           "Name","The Name of the Axis."
                           ); 

        this->methods()->addMethod( method( "isOn", &AxesComponent::isOn, this),
                        "Inspect the status of a Digital Input or Output.",
                        "Name", "The Name of the Digital IO."
                         );
        this->methods()->addMethod( method( "isEnabled", &AxesComponent::isEnabled, this),
                        "Inspect if an Axis is not locked.",
                        "Name", "The Name of the Axis."
                         );
        this->methods()->addMethod( method( "isLocked", &AxesComponent::isLocked, this),
                        "Inspect if an Axis is mechanically locked.",
                        "Name", "The Name of the Axis."
                         );
        this->methods()->addMethod( method( "isStopped", &AxesComponent::isStopped, this),
                        "Inspect if an Axis is electronically stopped.",
                        "Name", "The Name of the Axis."
                         );
        this->methods()->addMethod( method( "isDriven", &AxesComponent::isDriven, this),
                        "Inspect if an Axis is in movement.",
                        "Name", "The Name of the Axis."
                         );
        this->methods()->addMethod( method( "axes", &AxesComponent::getAxes, this),
                        "The number of axes."
                         );

        this->commands()->addCommand( command( "calibrateSensor", &AxesComponent::calibrateSensor, &AxesComponent::isCalibrated, this),
                                      "Calibrate a Sensor of an Axis.",
                                      "Axis", "The Name of the Axis.",
                                      "Sensor", "The Name of the Sensor of that axis e.g. 'Position'."
                                      );
        this->commands()->addCommand( command( "resetSensor", &AxesComponent::resetSensor, &AxesComponent::isCalibrated, this, true),
                                      "UnCalibrate a Sensor of an Axis.",
                                      "Axis", "The Name of the Axis.",
                                      "Sensor", "The Name of the Sensor of that axis e.g. 'Position'.");
        this->methods()->addMethod( method( "isSensorCalibrated", &AxesComponent::isCalibrated, this),
                                    "Check if a Sensor of an Axis is calibrated.",
                                    "Axis", "The Name of the Axis.",
                                    "Sensor", "The Name of the Sensor of that axis e.g. 'Position'.");
    }

    bool AxesComponent::startup()
    {
        if ( chan_sensor.connected() )
            {
                for (AxisMap::iterator it = axes.begin(); it != axes.end(); ++it)
                    {
                        // if an axis was added on a channel and its data port is connected
                        // as well, warn the user that the channel takes precedence.
                        if ( it->second.axis && it->second.inport->connected() && it->second.channel != -1 ) {
                            log(Warning) << "Axis: "<<it->second.name
                                         << ": Ignoring input from connected Data Port "
                                         << it->second.inport->getName() <<": using InputValues Data Port."<<endlog();
                        }
                    }
            }
        return true;
    }

    void AxesComponent::update()
    {
        /*
         * Access Drives, if a dataobject is present
         */
        if (usingChannels )
            chan_drive.Get( chan_out );

        /*
         * Process each axis.
         */
        for (AxisMap::iterator it= axes.begin(); it != axes.end(); ++it)
            this->to_axis( it->second );

        // Only write to channels if user requested so.
        if ( usingChannels )
            chan_sensor.Set( chan_meas );

    }

    void AxesComponent::shutdown()
    {
        for (AxisMap::iterator it= axes.begin(); it != axes.end(); ++it)
            it->second.axis->lock();
    }

    bool AxesComponent::addAxis( const std::string& name, AxisInterface* ax )
    {
        if ( this->isRunning() ) {
            log(Error) << "Can not add Axis "<<name<< " to running Component." <<endlog();
            return false;
        }
        if ( axes.count(name) != 0 ) {
            log(Error) << "Can not add Axis "<<name<< " to Component: name already in use." <<endlog();
            return false;
        }

        axes.insert( std::make_pair(name, AxisInfo(ax, name)));

        d_out[ name + ".Enable" ] = ax->getEnable();
        if ( ax->getBrake() )
            d_out[ name + ".Brake" ] = ax->getBrake();
        if ( ax->getSwitch("Home") )
           d_in[ name + ".Home" ] = ax->getSwitch("Home");

        // Add each sensor...
        AxisInfo* axinfo = & axes.find(name)->second;
        std::vector<std::string> res( ax->sensorList() );
        for ( std::vector<std::string>::iterator it = res.begin(); it != res.end(); ++it)
            {
                std::string sname( name+"_"+*it );
                axinfo->sensors[ sname ] = std::make_pair( ax->getSensor( *it ), new WriteDataPort<double>(sname) );
                this->ports()->addPort( axinfo->sensors[ sname ].second );
            }

        axinfo->inport = new DataPort<double>(name+"_Drive");
        this->ports()->addPort( axinfo->inport );
        return true;
    }

    bool AxesComponent::addAxisOnChannel(const std::string& axis_name, const std::string& sensor_name, int virtual_channel )
    {
        AxisInfo* axinfo = mhasAxis(axis_name);

        if ( axinfo == 0 || 
             virtual_channel >= max_channels ||
             axinfo->channel != -1 ||
             axinfo->axis->getSensor( sensor_name ) == 0 ||
             this->isRunning() )
            return false;

        // the channel reads drive values and provides values of one sensor.
        axinfo->channel = virtual_channel;
        axinfo->sensor = axinfo->axis->getSensor( sensor_name );

        if (usingChannels == 0) {
            this->ports()->addPort(&chan_sensor, "Sensor measurements.");
            this->ports()->addPort(&chan_drive, "Drive values.");
        }

        ++usingChannels;
        return true;
    }

    AxesComponent::AxisInfo* AxesComponent::mhasAxis(const std::string& axis_name)
    {
       AxisMap::iterator axit = axes.find(axis_name);
        if ( axit == axes.end() ) {
            log(Error) <<"No such axis present: "<< axis_name <<endlog();
            return 0;
        }
        return &axit->second;
    }

    void AxesComponent::removeAxisFromChannel( const std::string& axis_name )
    {
        AxisInfo* axinfo = mhasAxis(axis_name);
        if ( axinfo == 0 ||
             axinfo->channel == -1 ||
             this->isRunning() )
            return;

        axinfo->channel = -1;
        axinfo->sensor = 0;
        --usingChannels;

        if (usingChannels == 0) {
            this->ports()->removePort("InputValues");
            this->ports()->removePort("OutputValues");
        }

    }


    bool AxesComponent::removeAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 || this->isRunning() )
            return false;

        // cleanup the rest...
        d_out.erase( name + ".Enable" );
        d_out.erase( name + ".Brake" );
        d_in.erase( name + ".Home" );

        // Delete drive port.
        AxisInfo* axinfo = & axes.find(name)->second;
        this->ports()->removePort(axinfo->inport->getName());
        delete axinfo->inport;

        // Delete sensor ports.
        std::vector<std::string> res( axinfo->axis->sensorList() );
        for ( std::vector<std::string>::iterator it = res.begin(); it != res.end(); ++it)
            {
                std::string sname( name+"_"+*it );
                this->ports()->removePort(sname);
                delete axinfo->sensors[sname].second;
            }

        // erase the axis as last.
        axes.erase(name);
        return true;
    }

    bool AxesComponent::enableAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 )
            return false;
        AxisInfo* axinfo = & axes.find(name)->second;
        return axinfo->axis->unlock();
    }

    bool AxesComponent::stopAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 )
            return false;
        AxisInfo* axinfo = & axes.find(name)->second;
        return axinfo->axis->stop();
    }

    bool AxesComponent::disableAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 )
            return false;
        AxisInfo* axinfo = & axes.find(name)->second;
        return axinfo->axis->stop() && axinfo->axis->lock();
    }

    bool AxesComponent::switchOn( const std::string& name )
    {
        if (d_out.count(name) != 1)
            return false;
        d_out[name]->switchOn();
        return true;
    }
                    
    bool AxesComponent::switchOff( const std::string& name )
    {
        if (d_out.count(name) != 1)
            return false;
        d_out[name]->switchOff();
        return true;
    }

    bool AxesComponent::isEnabled( const std::string& name ) const
    {
        AxisMap::const_iterator it = axes.find(name);
        if ( it != axes.end() )
            return ! it->second.axis->isLocked(); // not locked == enabled
        return false;
    }

    bool AxesComponent::isLocked( const std::string& name ) const
    {
        AxisMap::const_iterator it = axes.find(name);
        if ( it != axes.end() )
            return it->second.axis->isLocked();
        return false;
    }

    bool AxesComponent::isStopped( const std::string& name ) const
    {
        AxisMap::const_iterator it = axes.find(name);
        if ( it != axes.end() )
            return it->second.axis->isStopped(); 
        return false;
    }

    bool AxesComponent::isDriven( const std::string& name ) const
    {
        AxisMap::const_iterator it = axes.find(name);
        if ( it != axes.end() )
            return it->second.axis->isDriven();
        return false;
    }

    int AxesComponent::getAxes() const
    {
        return axes.size();
    }

    bool AxesComponent::isOn( const std::string& name ) const
    {
        if ( d_in.count(name) == 1 )
            return d_in.find(name)->second->isOn();
        else if (d_out.count(name) == 1 )
            return d_out.find(name)->second->isOn();
        return false;
    }

    bool AxesComponent::calibrateSensor( const std::string& axis, const std::string& name )
    {
        AxisMap::iterator axit = axes.find(axis);
        if ( axit != axes.end() ) {
            SensorMap::iterator it = axit->second.sensors.find(name);
            if ( it != axit->second.sensors.end() )
                return it->second.first->calibrate(), true;
        }
        return false;
    }

    bool AxesComponent::resetSensor( const std::string& axis, const std::string& name )
    {
        AxisMap::iterator axit = axes.find(axis);
        if ( axit != axes.end() ) {
            SensorMap::iterator it = axit->second.sensors.find(name);
            if ( it != axit->second.sensors.end() )
                return it->second.first->unCalibrate(), true;
        }
        return false;
    }

    bool AxesComponent::isCalibrated( const std::string& axis, const std::string& name ) const
    {
        AxisMap::const_iterator axit = axes.find(axis);
        if ( axit != axes.end() ) {
            SensorMap::const_iterator it = axit->second.sensors.find(name);
            if ( it != axit->second.sensors.end() )
                return it->second.first->isCalibrated(), true;
        }
        return false;
    }

    void AxesComponent::to_axis(const AxisInfo& dd )
    {
        // first read all sensors.
        SensorMap::const_iterator it = dd.sensors.begin();
        while (it != dd.sensors.end() ) {
            it->second.second->Set( it->second.first->readSensor() );
            ++it;
        }

        // if a sensor was mapped to a channel, write that channel
        if (dd.channel != -1)
            chan_meas[dd.channel] = dd.sensor->readSensor();

        // if a drive was mapped to a channel, use that channel,
        // otherwise, read from data port.
        if ( dd.axis->isLocked() == false ) {
            if ( dd.channel == -1) {
                dd.axis->drive( dd.inport->Get() );
            }
            else
                dd.axis->drive( chan_out[dd.channel] );
        }
    }
}
