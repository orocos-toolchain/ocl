// Copyright (C) 2006 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
//                    Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
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
#include <rtt/Command.hpp>
#include <rtt/Event.hpp>

namespace Orocos
{
    /**
     * This class implements a TaskContext that can be used as
     * software Emergency Stop. It has a pointer to a
     * nAxesVelocityController and will call stopAllAxes and
     * lockAllAxes if needed.
     * Different events from different components can be added. If one
     * of the events is fired the nAxesVelocityController will be
     * stopped. 
     * 
     */

    class EmergencyStop
    {
    public:
        /** 
         * Constructor of the EmergencyStop
         * 
         * @param axes pointer to an nAxesVelocityController (GenericTaskContext)
         */
        EmergencyStop(RTT::GenericTaskContext* axes)
            : _axes(axes),fired(false)
        {
            _stop = axes->commands()->getCommand<bool(void)>("stopAllAxes");
            _lock = axes->commands()->getCommand<bool(void)>("lockAllAxes");
            if(!_stop.ready()||!_lock.ready()){
                log(Error)<<"(EmergencyStop) Stop and Lock Command are not ready"<<endlog();
            }
        };
        ~EmergencyStop(){};
        /** 
         * function to add an event that will fire the EmergencyStop
         * 
         * @param task pointer to the events' TaskContext
         * @param eventname name of the event
         * 
         * @return true if eventhandler could be constructed, false otherwise
         */
        bool addEvent(RTT::TaskContext* task,const std::string& eventname)
        {
            Handle handle;
            try{
                log(Debug)<<"Creating Handler"<<endlog();
                handle = task->events()->setupConnection(eventname).callback(this,&EmergencyStop::callback).handle();
            }catch(...){
                log(Error)<<"Could not create Handler for event "<<eventname<<endlog();
            }
            bool retval = false;
            try{
                log(Debug)<<"Connecting Event"<<endlog();
                retval = handle.connect();
                _handlers.push_back(handle);
            }catch(...){
                log(Error)<<"Could not connect event"<<eventname<<" to handler"<<endlog();
            }
            return retval;
        };
        /** 
         * Callback function, will be executed when one of the events
         * is fired.
         * 
         * @param message Message of the event, will be displayed in
         * EmergencyStop message
         */
        void callback(std::string message) {
            if(!fired){
                _stop();
                _lock();
                log(Error) << message <<endlog();
                log(Error) << "---------------------------------------------" << endlog();
                log(Error) << "--------- EMERGENCY STOP --------------------" << endlog();
                log(Error) << "---------------------------------------------" << endlog();
                fired=true;
            }
        };
    private:
        RTT::GenericTaskContext *_axes;
        RTT::Command<bool(void)> _stop;
        RTT::Command<bool(void)> _lock;
        std::vector<RTT::Handle> _handlers;
        bool fired;
    }; // class
}//namespace
