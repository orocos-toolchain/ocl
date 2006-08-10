namespace Orocos
{
    class EmergencyStop
    {
    public:
        EmergencyStop(RTT::GenericTaskContext* axes)
            : _axes(axes)
        {
            _stop = axes->commands()->getCommand<bool(void)>("stopAllAxes");
            _lock = axes->commands()->getCommand<bool(void)>("lockAllAxes");
            if(!_stop.ready()&&!_lock.ready()){
                log(Error)<<"(EmergencyStop) Stop and Lock Command are not ready"<<endlog();
            }
        };
        ~EmergencyStop(){};
        bool addEvent(RTT::GenericTaskContext* task,const std::string& eventname)
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
        }
        
        void callback() {
            _stop();
            _lock();
            log(Error) << "---------------------------------------------" << endlog();
            log(Error) << "--------- EMERGENCY STOP --------------------" << endlog();
            log(Error) << "---------------------------------------------" << endlog();
        };
    private:
        RTT::GenericTaskContext *_axes;
        RTT::Command<bool(void)> _stop;
        RTT::Command<bool(void)> _lock;
        std::vector<RTT::Handle> _handlers;
        
    }; // class
}//namespace
