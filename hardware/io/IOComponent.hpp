#ifndef ORO_IO_COMPONENT_HPP
#define ORO_IO_COMPONENT_HPP

#include <rtt/dev/AnalogInput.hpp>
#include <rtt/dev/DigitalInput.hpp>
#include <rtt/dev/AnalogOutput.hpp>
#include <rtt/dev/DigitalOutput.hpp>

#include <map>
#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <boost/tuple/tuple.hpp>
#include <rtt/Logger.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/DataPort.hpp>
#include <rtt/Property.hpp>
#include <rtt/RTT.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{

    /**
     * This Component uses the Orocos Device Interface for making IO
     * available to other components through data ports and methods.
     * - The 'InputValues' is what this component reads in from the hardware.
     * - The 'OutputValues' is what this component writes to the hardware.
     *
     * The component must be running in order to write values on its output
     * ports to hardware or providing read values on its input ports.
     *
     * It can handle the RTT::AnalogInInterface, RTT::AnalogOutInterface,
     * RTT::DigitalInInterface and RTT::DigitalOutInterface objects.
     * The analog IO is made available as Data ports and methods, while the digital IO
     * is available only through methods.
     *
     * The Comedi package contains implementations of these interfaces
     * and thus devices supported by comedi can be added to this component.
     *
     * This component uses the nameservers of the RTT::AnalogInInterface, RTT::AnalogOutInterface,
     * RTT::DigitalInInterface and RTT::DigitalOutInterface classes, thus you
     * must register your device with a name.
     */
    class IOComponent
        : public TaskContext
    {
    public:
        /**
         * @brief Create an IOComponent.
         */
        IOComponent(const std::string& name="IOComponent")
            :  TaskContext( name ),
               max_inchannels("MaximumInChannels","The maximum number of virtual analog input channels", 32),
               max_outchannels("MaximumOutChannels","The maximum number of virtual analog output channels", 32),
               inChannelPort( "InputValues", std::vector<double>( max_outchannels.get(), 0.0)  ),
               outChannelPort("OutputValues"),
               workvect(32),
               usingInChannels(0),
               usingOutChannels(0)
        {
            this->configure();

            this->ports()->addPort( &inChannelPort,
                                    "A DataPort containing the values read by this Component from hardware." );
            this->ports()->addPort( &outChannelPort,
                                    "A DataPort containing the values which this Component will write to hardware." );

            this->properties()->addProperty( &max_outchannels );
            this->properties()->addProperty( &max_inchannels );

            this->createAPI();
        }

        virtual bool configureHook()
        {
            if ( max_inchannels.get() > max_outchannels.get() )
                workvect.resize( max_inchannels.get() );
            else
                workvect.resize( max_outchannels.get() );

            inchannels.resize(max_inchannels.get(), 0 );
            outchannels.resize(max_outchannels.get(), 0 );
            chan_meas.resize(max_inchannels.get(), 0.0);
            chan_out.resize(max_outchannels.get(), 0);

            return true;
        }

        /**
         * First read the inputs, then write the outputs.
         */
        virtual void updateHook()
        {
            /*
             * Acces Analog device drivers
             */
            std::for_each( a_in.begin(), a_in.end(), bind( &IOComponent::write_a_in_to_do, this, _1 ) );
            std::for_each( ai_interface.begin(), ai_interface.end(), bind( &IOComponent::read_ai, this, _1 ) );

            if (usingInChannels) {
                for (unsigned int i=0; i < inchannels.size(); ++i)
                    chan_meas[i] = inchannels[i] ? inchannels[i]->value() : 0 ;

                // writeout.
                inChannelPort.Set( chan_meas );
            }

            /*
             * Acces Analog device drivers and Drives
             */
            std::for_each( a_out.begin(), a_out.end(), bind( &IOComponent::write_to_aout, this, _1 ) );
            std::for_each( ao_interface.begin(), ao_interface.end(), bind( &IOComponent::write_ao, this, _1 ) );

            // gather results.
            if ( usingOutChannels ) {
                outChannelPort.Get( chan_out );

                // writeout.
                for (unsigned int i=0; i < outchannels.size(); ++i)
                    if ( outchannels[i] )
                        outchannels[i]->value( chan_out[i] );
            }
        }

        /**
         * @brief Add an AnalogInInterface device interface.
         *
         * A DataPort is created which contains the value of all the
         * inputs.
         *
         * @param Portname The name of the interface, which will also be given to the DataPort.
         * @param devicename   The AnalogInInterface for reading the input.
         */
        bool addAnalogInInterface( const std::string& Portname, const std::string& devicename)
        {
            Logger::In("addAnalogInInterface");
            AnalogInInterface* input =  AnalogInInterface::nameserver.getObject(devicename);
            if ( !input ) {
                log(Error) << "AnalogIn: "<< devicename <<" not found."<<endlog();
                return false;
            }
            if ( ai_interface.count(Portname) != 0 ){
                log(Error) << "Portname: "<< Portname <<" already in use."<<endlog();
                return false;
            }
            if ( isRunning() ) {
                log(Error) << "Can not add interfaces when running."<<endlog();
                return false;
            }

            ai_interface[Portname] =
                boost::make_tuple(input, new WriteDataPort<std::vector<double> >(Portname, std::vector<double>(input->nbOfChannels())) );

            this->ports()->addPort( boost::get<1>(ai_interface[Portname]), "Analog Input value.");

            this->exportPorts();
            return true;
        }

        /**
         * @brief Remove an AnalogInInterface device interface.
         *
         * The DataPort  which contains the value of all the
         * inputs is removed.
         *
         * @param Portname The name of the interface to remove
         */
        bool removeAnalogInInterface(const std::string& Portname)
        {
            Logger::In("removeAnalogInInterface");
            if ( ai_interface.count(Portname) == 0 ){
                log(Error) << "Portname: "<< Portname <<" unknown."<<endlog();
                return false;
            }
            if ( isRunning() ) {
                log(Error) << "Can not remove interfaces when running."<<endlog();
                return false;
            }

            delete boost::get<1>(ai_interface[Portname]);
            this->removeObject( Portname );
            ai_interface.erase(Portname);
            return true;
        }

        /**
         * @brief Add an AnalogInput device.
         *
         * A DataPort is created which contains
         * the converted value of the input. The raw value of the channel can be read
         * through the DataPort Portname + "_raw".
         *
         *
         * @param Portname The name of the DataObject.
         * @param devicename   The AnalogInInterface for reading the input.
         * @param channel The channel of the input.
         */
        bool addAnalogInput( const std::string& Portname, const std::string& devicename, int channel)
        {
            if ( a_in.count(Portname) != 0  || this->isRunning() )
                return false;

            AnalogInInterface* input =  AnalogInInterface::nameserver.getObject(devicename);
            if ( !input ) {
                log(Error) << "AnalogIn: "<< devicename <<" not found."<<endlog();
                return false;
            }

            a_in[Portname] =
                boost::make_tuple( new AnalogInput( input, channel ),
                            new WriteDataPort<unsigned int>(Portname+"_raw", 0),
                            new WriteDataPort<double>(Portname, 0.0) );

            this->ports()->addPort( boost::get<1>(a_in[Portname]), "Analog Input raw value.");
            this->ports()->addPort( boost::get<2>(a_in[Portname]), "Analog Input value.");

            this->exportPorts();
            return true;
        }

        /**
         * @brief Remove a previously added AnalogInput.
         */
        bool removeAnalogInput( const std::string& Portname )
        {
            if ( a_in.count(Portname) != 1 || this->isRunning() )
                return false;

            using namespace boost;
            tuple<AnalogInput*, WriteDataPort<unsigned int>*, WriteDataPort<double>* > res = a_in[Portname];
            this->ports()->removePort( Portname );
            this->ports()->removePort( Portname );
            a_in.erase(Portname);

            delete get<0>(res);
            delete get<1>(res);
            delete get<2>(res);

            this->removeObject(Portname);
            return true;
        }

        /**
         * @brief Add an analog Channel to InputValues
         *
         * A std::vector<double> DataPort ( "InputValues") is used
         * which contains the converted value of the input.
         *
         * @param virt_channel The virtual channel (in software).
         * @param devicename   The AnalogInInterface for reading the input.
         * @param channel The physical channel of the input (in hardware).
         */
        bool addInputChannel(int virt_channel, const std::string& devicename, int channel )
        {
            if ( inchannels[virt_channel] != 0 || this->isRunning() )
                return false;

            AnalogInInterface* input =  AnalogInInterface::nameserver.getObject(devicename);
            if ( !input ) {
                log(Error) << "AnalogIn: "<< devicename <<" not found."<<endlog();
                return false;
            }
            ++usingInChannels;

            inchannels[virt_channel] = new AnalogInput( input, channel ) ;

            return true;
        }

        /**
         * @brief Remove a previously added analog Channel.
         */
        bool removeInputChannel( int virt_channel )
        {
            if ( this->isRunning() )
                return false;

            --usingInChannels;

            inchannels[virt_channel] = 0;

            return true;
        }

        /**
         * @brief Add an DigitalInput device.
         *
         * No DataObject is created, but the input is available through the execution
         * interface.
         *
         * @param name    The name of the Digital Input.
         * @param devicename   The DigitalInInterface for reading the input.
         * @param channel The channel of the input.
         * @param invert  True if the input must be inverted, false(default) otherwise.
         */
        bool addDigitalInput( const std::string& name, const std::string& devicename, int channel, bool invert=false)
        {
            if ( d_in.count(name) != 0 || this->isRunning() )
                return false;

            DigitalInInterface* input =  DigitalInInterface::nameserver.getObject(devicename);
            if ( !input ) {
                log(Error) << "DigitalIn: "<< devicename <<" not found."<<endlog();
                return false;
            }

            d_in[name] = new DigitalInput( input, channel, invert );

            return true;
        }

        /**
         * @brief Remove a previously added Digital Input.
         */
        bool removeDigitalInput( const std::string& name )
        {
            if ( d_in.count(name) != 1 || this->isRunning() )
                return false;

            DigitalInput* res = d_in[name];

            delete res;

            d_in.erase(name);
            return true;
        }

        /**
         * @brief Add an AnalogOutInterface device interface.
         *
         * A DataPort is created which contains the value of all the
         * inputs.
         *
         * @param Portname The name of the interface, which will also be given to the DataPort.
         * @param devicename   The AnalogOutInterface for reading the input.
         */
        bool addAnalogOutInterface( const std::string& Portname, const std::string& devicename)
        {
            Logger::In("addAnalogOutInterface");
            AnalogOutInterface* output =  AnalogOutInterface::nameserver.getObject(devicename);
            if ( !output ) {
                log(Error) << "AnalogOut: "<< devicename <<" not found."<<endlog();
                return false;
            }
            if ( ao_interface.count(Portname) != 0 ){
                log(Error) << "Portname: "<< Portname <<" already in use."<<endlog();
                return false;
            }
            if ( isRunning() ) {
                log(Error) << "Can not add interfaces when running."<<endlog();
                return false;
            }

            ao_interface[Portname] =
                std::make_pair(output, new ReadDataPort<std::vector<double> >(Portname) );

            this->ports()->addPort( ao_interface[Portname].second, "Analog Output value.");

            this->exportPorts();
            return true;
        }

        /**
         * @brief Remove an AnalogOutInterface device interface.
         *
         * The DataPort  which contains the value of all the
         * inputs is removed.
         *
         * @param Portname The name of the interface to remove
         */
        bool removeAnalogOutInterface(const std::string& Portname)
        {
            Logger::In("removeAnalogOutInterface");
            if ( ao_interface.count(Portname) == 0 ){
                log(Error) << "Portname: "<< Portname <<" unknown."<<endlog();
                return false;
            }
            if ( isRunning() ) {
                log(Error) << "Can not remove interfaces when running."<<endlog();
                return false;
            }

            delete ao_interface[Portname].second;
            this->removeObject( Portname );
            ao_interface.erase(Portname);
            return true;
        }

        /**
         * @brief Add an AnalogOutput which reads from an Output DataObject.
         *
         * @param portname    The portname of the DataObject to read.
         * @param devicename  The Analog Device to write to.
         * @param channel The channel of the Device to write to.
         *
         * @return true on success, false otherwise
         */
        bool addAnalogOutput( const std::string& portname, const std::string& devicename, int channel )
        {
            if ( a_out.count(portname) != 0 || this->isRunning() )
                return false;

            AnalogOutInterface* output =  AnalogOutInterface::nameserver.getObject(devicename);
            if ( !output ) {
                log(Error) << "AnalogOut: "<< devicename <<" not found."<<endlog();
                return false;
            }

            a_out[portname] = std::make_pair( new AnalogOutput( output, channel ), new ReadDataPort<double>(portname) );

            this->ports()->addPort( a_out[portname].second, "Analog Output value." );

            this->exportPorts();
            return true;
        }

        /**
         * @brief Remove a previously added AnalogOutput
         *
         * @param name The name of the DataObject to which it was connected
         *
         * @return true on success, false otherwise
         */
        bool removeAnalogOutput( const std::string& name )
        {
            if ( a_out.count(name) != 1 || this->isRunning() )
                return false;

            this->ports()->removePort( name );

            delete a_out[name].first;
            delete a_out[name].second;

            a_out.erase(name);
            this->removeObject(name);
            return true;
        }

        /**
         * @brief Add a virtual channel to OutputValues for writing an analog value.
         *
         * A std::vector<double> DataPort ( "OutputValues") is used
         * to which the value of the output can be written.
         *
         * @param virt_channel The position in OutputValues
         * @param devicename   The Device to write the data to.
         * @param channel      The channel of the Device to use.
         *
         * @return true on success, false otherwise.
         */
        bool addOutputChannel( int virt_channel, const std::string& devicename, int channel )
        {
            if ( outchannels[virt_channel] != 0 || this->isRunning() )
                return false;

            AnalogOutInterface* output =  AnalogOutInterface::nameserver.getObject(devicename);
            if ( !output ) {
                log(Error) << "AnalogOut: "<< devicename <<" not found."<<endlog();
                return false;
            }


            outchannels[virt_channel] = new AnalogOutput( output, channel );
            ++usingOutChannels;
            return true;
        }

        /**
         * @brief Remove the use of a virtual channel
         *
         * @param virt_channel The number of the channel to remove
         *
         */
        bool removeOutputChannel( int virt_channel )
        {
            if ( outchannels[virt_channel] == 0 || this->isRunning() )
                return false;

            delete outchannels[virt_channel];
            --usingOutChannels;
            return true;
        }

        /**
         * @brief Add a complete DigitalInInterface.
         *
         * @param name    The base name of the DigitalInputs. Their name will be appended with a number.
         * @param devicename  The Device to read from.
         *
         */
        bool addDigitalInInterface( const std::string& name, const std::string& devicename)
        {
            if ( this->isRunning() )
                return false;

            DigitalInInterface* input =  DigitalInInterface::nameserver.getObject(devicename);
            if ( !input ) {
                log(Error) << "DigitalInput: "<< devicename <<" not found."<<endlog();
                return false;
            }

            std::stringstream name_number;
            for(unsigned int i=0; i != input->nbOfInputs(); ++i) {
                name_number << name;
                name_number << i;
                if ( d_in.count(name_number.str()) )
                    delete d_in.find(name_number.str())->second;
                d_in[name_number.str()] = new DigitalInput( input, i );
                name_number.str("");
            }

            return true;
        }

        /**
         * @brief Add a complete DigitalOutInterface.
         *
         * @param name    The base name of the DigitalOutputs. Their name will be appended with a number.
         * @param devicename  The Device to write to.
         *
         */
        bool addDigitalOutInterface( const std::string& name, const std::string& devicename)
        {
            if ( this->isRunning() )
                return false;

            DigitalOutInterface* output =  DigitalOutInterface::nameserver.getObject(devicename);
            if ( !output ) {
                log(Error) << "DigitalOut: "<< devicename <<" not found."<<endlog();
                return false;
            }

            std::stringstream name_number;
            for(unsigned int i=0; i != output->nbOfOutputs(); ++i) {
                name_number << name;
                name_number << i;
                if ( d_out.count(name_number.str()) )
                    delete d_out.find(name_number.str())->second;
                d_out[name_number.str()] = new DigitalOutput( output, i );
                name_number.str("");
            }

            return true;
        }

        /**
         * @brief Remove a complete DigitalInInterface.
         *
         * @param name    The base name of the DigitalInputs to remove.
         *
         */
        bool removeDigitalInInterface( const std::string& name)
        {
            if ( this->isRunning() )
                return false;

            std::stringstream name_number;
            for(int i=0; i != 128; ++i) { // FIXME: magic number.
                name_number << name;
                name_number << i;
                if ( d_in.count(name_number.str()) )
                    delete d_in.find(name_number.str())->second;
                d_in.erase(name_number.str());
                name_number.str("");
            }

            return true;
        }

        /**
         * @brief Remove a complete DigitalOutInterface.
         *
         * @param name    The base name of the DigitalOutputs to remove.
         *
         */
        bool removeDigitalOutInterface( const std::string& name)
        {
            if ( this->isRunning() )
                return false;

            std::stringstream name_number;
            for(int i=0; i != 128; ++i) { // FIXME: magic number.
                name_number << name;
                name_number << i;
                if ( d_out.count(name_number.str()) )
                    delete d_out.find(name_number.str())->second;
                d_out.erase(name_number.str());
                name_number.str("");
            }

            return true;
        }

        /**
         * @brief Add a single DigitalOutput.
         *
         * @param name    The name of the DigitalOutput.
         * @param devicename  The Device to write to.
         * @param channel The channel/bit of the device to use
         * @param invert  Invert the output or not.
         *
         */
        bool addDigitalOutput( const std::string& name, const std::string& devicename, int channel, bool invert=false)
        {
            if ( d_out.count(name) != 0 || this->isRunning() )
                return false;

            DigitalOutInterface* output =  DigitalOutInterface::nameserver.getObject(devicename);
            if ( !output ) {
                log(Error) << "DigitalOut: "<< devicename <<" not found."<<endlog();
                return false;
            }

            d_out[name] = new DigitalOutput( output, channel, invert );

            return true;
        }

        /**
         * @brief Remove a previously added DigitalOutput
         *
         * @param name The name of the DigitalOutput
         *
         */
        bool removeDigitalOutput( const std::string& name )
        {
            if ( d_out.count(name) != 1 || this->isRunning() )
                return false;

            delete d_out[name];

            d_out.erase(name);
            return true;
        }

        /**
         * @name Scripting Methods
         * Runtime methods to inspect the IO values.
         * @{
         */
        void switchOn( const std::string& name )
        {
            DOutMap::iterator it = d_out.find(name);
            if ( it == d_out.end() )
                return;
            it->second->switchOn();
        }

        /**
         * @brief Is a DigitalOutput or DigitalInput on ?
         *
         * @param name The DigitalOutput or DigitalInput to inspect.
         *
         * @return true if on, false otherwise.
         */
        bool isOn( const std::string& name ) const
        {
            DOutMap::const_iterator it = d_out.find(name);
            if ( it != d_out.end() )
                return it->second->isOn();
            DInMap::const_iterator it2 = d_in.find(name);
            if ( it2 != d_in.end() )
                return it2->second->isOn();
            return false;
        }

        /**
         * @brief Switch off a DigitalOutput.
         *
         * @param name The name of the output to switch off.
         */
        void switchOff( const std::string& name )
        {
            DOutMap::const_iterator it = d_out.find(name);
            if ( it == d_out.end() )
                return;
            it->second->switchOff();
        }

        /**
         * @brief Return the value of an AnalogInput or AnalogOutput
         *
         * @param name The name of the AnalogInput or AnalogOutput
         *
         * @return The physical value.
         */
        double value(const std::string& name) const
        {
            AInMap::const_iterator it = a_in.find(name);
            if ( it != a_in.end() )
                return boost::get<0>( it->second )->value();
            AOutMap::const_iterator it2 = a_out.find(name);
            if ( it2 != a_out.end() )
                return it2->second.first->value();
            return 0;
        }

        /**
         * @brief Return the raw sensor value of an AnalogInput or AnalogOutput
         *
         * @param name The name of the AnalogInput or AnalogOutput
         *
         * @return The raw value.
         */
        int rawValue(const std::string& name) const
        {
            AInMap::const_iterator it = a_in.find(name);
            if ( it != a_in.end() )
                return boost::get<0>( it->second )->rawValue();

            AOutMap::const_iterator it2 = a_out.find(name);
            if ( it2 != a_out.end() )
                return it2->second.first->rawValue();
            return 0;
        }

        /**
         * Return the number of Channels this component reads from.
         *
         */
        int getInputChannels() const
        {
            return chan_meas.size();
        }

        /**
         * Return the number of Channels this component writes to.
         *
         */
        int getOutputChannels() const
        {
            return chan_out.size();
        }
        /**
         * @}
         */

    protected:

        void createAPI();

        Property<int> max_inchannels;
        Property<int> max_outchannels;
        /**
         * The 'Channels' provide an array of measured analog inputs.
         */
        std::vector< AnalogInput* > inchannels;
        std::vector< AnalogOutput* > outchannels;

        std::vector<double> chan_meas;
        std::vector<double> chan_out;
        WriteDataPort< std::vector<double> > inChannelPort;
        ReadDataPort< std::vector<double> > outChannelPort;

        /**
         * Each digital input/output becomes an object with methods in the component interface.
         */
        typedef std::map<std::string, DigitalInput* > DInMap;
        DInMap d_in;
        typedef std::map<std::string, DigitalOutput* > DOutMap;
        DOutMap d_out;

        /**
         * Each analog input/output becomes a port and a raw_port in the component interface.
         */
        typedef
        std::map<std::string,
                 boost::tuple< AnalogInput*,
                        WriteDataPort<unsigned int>*,
                        WriteDataPort<double>* > > AInMap;
        AInMap a_in;
        typedef std::map<std::string, std::pair<AnalogOutput*, ReadDataPort<double>* > > AOutMap;
        AOutMap a_out;

        typedef
        std::map<std::string,
                 boost::tuple< AnalogInInterface*,
                               WriteDataPort<std::vector<double> >* > > AInInterfaceMap;
        AInInterfaceMap ai_interface;
        typedef std::map<std::string, std::pair<AnalogOutInterface*, ReadDataPort<std::vector<double> >* > > AOutInterfaceMap;
        AOutInterfaceMap ao_interface;
        std::vector<double> workvect;

        int usingInChannels;
        int usingOutChannels;

        /**
         * Write Analog input to DataObject.
         */
        void write_a_in_to_do( const AInMap::value_type& dd )
        {
            // See boost::tuple for syntax
            boost::get<1>(dd.second)->Set( boost::get<0>(dd.second)->rawValue() );
            boost::get<2>(dd.second)->Set( boost::get<0>(dd.second)->value() );
        }

        /**
         * Write to Data to digital output.
         */
        void write_to_aout( const AOutMap::value_type& dd )
        {
            dd.second.first->value( dd.second.second->Get() );
        }

        /**
         * Write Analog input to DataObject.
         */
        void read_ai( const AInInterfaceMap::value_type& dd )
        {
            // See boost::tuple for syntax
            workvect.resize( boost::get<0>(dd.second)->nbOfChannels() );
            for(unsigned int i=0; i != workvect.size(); ++i) {
                AnalogInput ain( boost::get<0>(dd.second), i);
                workvect[i] = ain.value();
            }
            boost::get<1>(dd.second)->Set( workvect );
        }

        void write_ao( const AOutInterfaceMap::value_type& dd )
        {
            workvect.resize( dd.second.first->nbOfChannels() );
            dd.second.second->Get( workvect );
            for(unsigned int i=0; i != workvect.size(); ++i) {
                AnalogOutput aout( dd.second.first, i);
                aout.value( workvect[i]);
            }
        }

    };

}

#endif

