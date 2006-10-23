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

namespace Orocos 
{

    AxesComponent::AxesComponent( int max_chan , const std::string& name ) 
        :  TaskContext(name),
           max_channels("MaximumChannels","The maximum number of virtual sensor/drive channels", max_chan),
           usingChannels(0)
    {
        AxisInterface* _a = 0;
        SensorInterface<double>* _d = 0;
        channels.resize(max_chan, std::make_pair(_d,_a) );
        chan_meas.resize(max_chan, 0.0);

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
        this->methods()->addMethod( method( "position", &AxesComponent::position, this),
                        "Inspect the status of the Position of an Axis.",
                        "Name", "The Name of the Axis."
                         );
        this->methods()->addMethod( method( "readSensor", &AxesComponent::readSensor, this),
                        "Inspect the status of a Sensor of an Axis.",
                        "FullName", "The Name of the Axis followed by a '::' and the Sensor name (e.g. 'Position'."
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
                        "FullName", "The Name of the Axis followed by a '::' and the Sensor name (e.g. 'Position'."
                        );
        this->commands()->addCommand( command( "resetSensor", &AxesComponent::resetSensor, &AxesComponent::isCalibrated, this),
                        "UnCalibrate a Sensor of an Axis.",
                        "FullName", "The Name of the Axis followed by a '::' and the Sensor name (e.g. 'Position'."
                        ,true);
    }

    bool AxesComponent::componentLoaded()
    {
        if ( !Input->dObj()->Get("ChannelValues",chan_DObj) )
            return false;
        // kind-of resize of the vector in the dataobject:
        chan_DObj->Set(chan_meas); 

        // Get All DataObjects of Added Axes
        for(AxisMap::iterator ax= axes.begin(); ax != axes.end(); ++ax)
            {
                if ( axis_to_remove == ax->first )
                    continue; // detect removal of axis...
                DataObjectInterface<double>* d;
                if ( this->Input->dObj()->Get( ax->first+"_Velocity", d) == false )
                    std::cout << "AxesComponent::componentLoaded : Velocity of "+ax->first+" not found !"<<std::endl;

                drive[ ax->first ] = std::make_pair( ax->second, d );

                std::vector<std::string> res( ax->second->sensorList() );
                for ( std::vector<std::string>::iterator it = res.begin(); it != res.end(); ++it)
                    {
                        std::string sname( ax->first+"_"+*it );
                        if ( this->Input->dObj()->Get( sname, d ) == false )
                            std::cout << "AxesComponent::componentLoaded : "+*it+" of "+ax->first+" not found !"<<std::endl;
                    }
            }

        return true;
    }

    void AxesComponent::componentUnloaded() {
        // Delete all Data Objects is done by DataObject Server
        // just cleanup stuff in opposite to componentLoaded
        for(AxisMap::iterator ax= axes.begin(); ax != axes.end(); ++ax)
            {
                drive.erase( ax->first );

                std::vector<std::string> res( ax->second->sensorList() );
                for ( std::vector<std::string>::iterator it = res.begin(); it != res.end(); ++it)
                    {
                        std::string sname( ax->first + "_"+*it );
                        sensor.erase( sname );
                    }
            }
    }

    void AxesComponent::update()
    {
        /*
         * Access Sensor device drivers
         */
        std::for_each( sensor.begin(), sensor.end(), bind( &AxesComponent::sensor_to_do, this, _1 ) );

        // Only write to channels if user requested so.
        if ( usingChannels ) {
            for (unsigned int i=0; i < channels.size(); ++i)
                chan_meas[i] = channels[i].first ? channels[i].first->readSensor() : 0 ;

            // writeout.
            chan_sensor->Set( chan_meas );
        }

        /*
         * Access Drives, if a dataobject is present
         */
        std::for_each( drive.begin(), drive.end(), bind( &AxesComponent::write_to_drive, this, _1 ) );

        /* Access Drives if the user uses 'ChannelValues' */
        if (usingChannels ) {
            chan_drive->Get( chan_out );

            // writeout.
            for (unsigned int i=0; i < channels.size(); ++i) {
                if ( channels[i].second ) {
                    channels[i].second->drive( chan_out[i] );
                }
            }
        }

    }

    bool AxesComponent::addAxis( const std::string& name, AxisInterface* ax )
    {
        if ( this->isRunning() ) {
            log(Error) << "Can not add Axis "<<name<< " to running Component." <<endlog();
            return false;
        }
        if ( axes[name] ) {
            log(Error) << "Can not add Axis "<<name<< " to Component: name already in use." <<endlog();
            return false;
        }

        axes[name] = ax;

        d_out[ name + ".Enable" ] = ax->getDrive()->enableGet();
        if ( ax->getBrake() )
            d_out[ name + ".Brake" ] = ax->getBrake();
        if ( ax->getSwitch("Home") )
           d_in[ name + ".Home" ] = ax->getSwitch("Home");


        // Add each sensor...
        std::vector<std::string> res( ax->sensorList() );
        for ( std::vector<std::string>::iterator it = res.begin(); it != res.end(); ++it)
            {
                std::string sname( name+"_"+*it );
                sensor[ sname ] = std::make_pair( ax->getSensor( *it ), new WriteDataPort<double>(sname) );
            }

        drive[name + "_Drive" ] = make_pair( ax, new ReadDataPort<double>(name) );
        return true;
    }

    bool AxesComponent::addAxisOnChannel(const std::string& axis_name, const std::string& sensor_name, int virtual_channel )
    {
        if ( virtual_channel >= max_channels ||
             channels[virtual_channel].first != 0 ||
             axes.count(axis_name) != 1 ||
             axes[axis_name]->getSensor( sensor_name ) == 0 ||
             this->isRunning() )
            return false;

        // The owner Axis is stored in the channel.
        ++usingChannels;
        channels[virtual_channel] = std::make_pair( axes[axis_name]->getSensor( sensor_name ), axes[axis_name] );
        return true;
    }

    void AxesComponent::removeAxisFromChannel( int virtual_channel )
    {
        if ( virtual_channel >= max_channels ||
             virtual_channel < 0 ||
             channels[virtual_channel].first == 0 ||
             this->isRunning() )
            return;

        --usingChannels;

        AxisInterface* _a = 0;
        SensorInterface<double>* _d = 0;
        // Reset the channel
        channels[virtual_channel] = std::make_pair( _d, _a);
    }


    bool AxesComponent::removeAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 || this->isRunning() )
            return false;

        for ( std::vector< std::pair< const SensorInterface<double>*, AxisInterface* > >::iterator it = channels.begin();
              it != channels.end();
              ++it )
            if ( it->second == axes[name] )
                {
                    it->first = 0; // clear the channel occupied by an axis sensor
                    it->second = 0;
                }

        // cleanup the rest...
        d_out.erase( name + ".Enable" );
        d_out.erase( name + ".Brake" );
        d_in.erase( name + ".Home" );

        // Delete drive port.
        delete drive[name+"_Drive"].second;
        drive.erase( name + "_Drive");

        // Delete sensor ports.
        std::vector<std::string> res( axes[name]->sensorList() );
        for ( std::vector<std::string>::iterator it = res.begin(); it != res.end(); ++it)
            {
                std::string sname( name+"_"+*it );
                delete sensor[sname].second;
                sensor.erase(sname);
            }

        // erase the axis as last.
        axes.erase(name);
        return true;
    }

    bool AxesComponent::enableAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 )
            return false;
        return axes[name].first->unlock();
    }

    bool AxesComponent::stopAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 )
            return false;
        return axes[name].first->stop();
    }

    bool AxesComponent::disableAxis( const std::string& name )
    {
        if ( axes.count(name) != 1 )
            return false;
        return axes[name].first->stop() && axes[name].first->lock();
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
            return ! it->second->isLocked(); // not locked == enabled
        return false;
    }

    bool AxesComponent::isLocked( const std::string& name ) const
    {
        AxisMap::const_iterator it = axes.find(name);
        if ( it != axes.end() )
            return it->second->isLocked();
        return false;
    }

    bool AxesComponent::isStopped( const std::string& name ) const
    {
        AxisMap::const_iterator it = axes.find(name);
        if ( it != axes.end() )
            return it->second->isStopped(); 
        return false;
    }

    bool AxesComponent::isDriven( const std::string& name ) const
    {
        AxisMap::const_iterator it = axes.find(name);
        if ( it != axes.end() )
            return it->second->isDriven();
        return false;
    }

    int AxesComponent::getAxes() const
    {
        return axes.size();
    }

    double AxesComponent::position( const std::string& name ) const
    {
        SensorMap::const_iterator it = sensor.find(name+"_Position");
        if ( it != sensor.end() )
            return it->second.first->readSensor();
        return 0;
    }

    double AxesComponent::readSensor( const std::string& name ) const
    {
        SensorMap::const_iterator it = sensor.find(name);
        if ( it != sensor.end() )
            return it->second.first->readSensor();
        return 0;
    }

    bool AxesComponent::isOn( const std::string& name ) const
    {
        if ( d_in.count(name) == 1 )
            return d_in.find(name)->second->isOn();
        else if (d_out.count(name) == 1 )
            return d_out.find(name)->second->isOn();
        return false;
    }

    bool AxesComponent::calibrateSensor( const std::string& name )
    {
        SensorMap::iterator it = sensor.find(name);
        if ( it != sensor.end() )
            return it->second.first->calibrate(), true;
        return false;
    }

    bool AxesComponent::resetSensor( const std::string& name )
    {
        SensorMap::iterator it = sensor.find(name);
        if ( it != sensor.end() )
            return it->second.first->unCalibrate(), true;
        return false;
    }

    bool AxesComponent::isCalibrated( const std::string& name ) const
    {
        SensorMap::const_iterator it = sensor.find(name);
        if ( it != sensor.end() )
            return it->second.first->isCalibrated();
        return false;
    }

    void AxesComponent::sensor_to_do( const std::pair<std::string,std::pair< const SensorInterface<double>*,
                                      WriteDataPort<double>* > >& dd )
    {
        dd.second.second->Set( dd.second.first->readSensor() );
    }

    void AxesComponent::write_to_drive( pair<std::string, pair<AxisInterface*, ReadDataPort<double>* > > dd )
    {
        if ( dd.second.second != 0)
            dd.second.first->drive( dd.second.second->Get() );
    }

}
