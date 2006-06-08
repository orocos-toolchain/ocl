/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:26 CET 2004  ReportingComponent.hpp 

                        ReportingComponent.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
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
 
#ifndef ORO_REPORTING_COMPONENT_HPP
#define ORO_REPORTING_COMPONENT_HPP


#include <os/main.h>
#include <boost/tuple/tuple.hpp>

#include <corelib/Property.hpp>
#include <corelib/PropertyBag.hpp>
#include <corelib/Marshaller.hpp>
#include <corelib/TimeService.hpp>
#include <corelib/CommandInterface.hpp>
#include <execution/GenericTaskContext.hpp>

// Impl.
#include <execution/TemplateFactories.hpp>
#include <corelib/marshalling/TableMarshaller.hpp>
#include <corelib/marshalling/TableHeaderMarshaller.hpp>
#include <corelib/marshalling/EmptyMarshaller.hpp>
#include <iostream>

namespace ORO_Components
{
    /**
     * @brief A Component for periodically reporting Component
     * DataObject contents to a human readable text format. The
     * default is a table with a header.
     *
     * It can report to any data format, using the 'addMarshaller'
     * function.
     */
    class ReportingComponent
        : public ORO_Execution::GenericTaskContext
    {
    public:

        typedef ORO_Execution::DataFlowInterface::Ports Ports;

        /**
         * Set up a component for reporting.
         */
        ReportingComponent( std::string name = "Reporting" ) 
            : ORO_Execution::GenericTaskContext( name ),
              report("Report"),
              autotrigger("AutoTrigger","When set to 1, the data is taken upon each update, "
                          "otherwise, the data is only taken when the user invokes 'trigger()'.",
                          true),
              starttime(0), timestamp("TimeStamp","The time at which the data was read.",0.0)
        {
            this->attributes()->addProperty( &autotrigger );
            this->attributes()->addProperty( &autotrigger );

            using namespace ORO_Execution;
            // Add the methods, methods make sure that they are 
            // executed in the context of the (non realtime) caller.
            TemplateMethodFactory< ReportingComponent  >* ret =
                newMethodFactory( this );
            ret->add( "snapshot",
                      method
                      ( &ReportingComponent::snapshot ,
                        "Take a new shapshot of the data and set the timestamp.") );
            ret->add( "reportComponent",
                      method
                      ( &ReportingComponent::reportComponent ,
                        "Add a Component for reporting. Only works if Component is connected.",
                        "Component", "Name of the Component") );
            ret->add( "unreportComponent",
                      method
                      ( &ReportingComponent::unreportComponent ,
                        "Remove a Component from reporting.",
                        "Component", "Name of the Component"
                        ) );
            ret->add( "reportData",
                      method
                      ( &ReportingComponent::reportData ,
                        "Add a Component's DataSource for reporting. Only works if DataObject exists and Component is connected.",
                        "Component", "Name of the Component",
                        "DataObject", "Name of the DataObject. For example, a property or attribute.") );
            ret->add( "unreportData",
                      method
                      ( &ReportingComponent::unreportData ,
                        "Remove a DataObject from reporting.",
                        "Component", "Name of the Component",
                        "DataObject", "Name of the DataObject.") );
            ret->add( "reportConnection",
                      method
                      ( &ReportingComponent::reportConnection ,
                        "Add a Component's Connection for reporting. Only works if the Connection exists and Component is connected.",
                        "Component", "Name of the Component",
                        "Port", "Name of the Port to the connection.") );
            ret->add( "unreportConnection",
                      method
                      ( &ReportingComponent::unreportConnection ,
                        "Remove a Connection for reporting.",
                        "Component", "Name of the Component",
                        "Port", "Name of the Port to the connection.") );
            this->methods()->registerObject("this", ret);
        }

        virtual ~ReportingComponent() {
        }

        /**
         * Adds a Plugin to receive incomming data. If the user has not used this
         * method, a default pair will be created upon startup().
         * @param header A marshaller which writes out a header when this
         * component is started. May be null (0).
         * @param body A marshaller wich will get periodically a serialisation
         * request to process incomming data. May be null(0).
         * example:
         * addMarshaller( new HeaderMarshaller(), new ContentsMarshaller() );
         *
         */
        bool addMarshaller( ORO_CoreLib::Marshaller* headerM, ORO_CoreLib::Marshaller* bodyM)
        {
            boost::shared_ptr<ORO_CoreLib::Marshaller> header(headerM);
            boost::shared_ptr<ORO_CoreLib::Marshaller> body(bodyM);
            if ( !header && !body)
                return false;
            if ( !header )
                header.reset( new ORO_CoreLib::EmptyMarshaller() );
            if ( !body)
                body.reset( new ORO_CoreLib::EmptyMarshaller());

            marshallers.push_back( std::make_pair( header, body ) );
        }

        /**
         * @name Script Methods
         * @{
         */
        // write interface to file+values of prop/attrs.
        bool screenComponent( const std::string& comp ) { return false; }

        // report the datasources.
        bool reportComponent( const std::string& component ) { 
            // Users may add own data sources, so avoid duplicates
            //std::vector<std::string> sources                = comp->data()->getNames();
            ORO_Execution::TaskContext* comp = this->getPeer(component);
            using namespace ORO_CoreLib;
            if ( !comp ) {
                Logger::log() <<Logger::Error << "Could not report Component " << component <<" : no such peer."<<Logger::endl;
                return false;
            }
            Ports ports   = comp->ports()->getPorts();
            for (Ports::iterator it = ports.begin(); it != ports.end() ; ++it) {
                if ( (*it)->connected() ) {
                    this->reportDataSource( component + "." + (*it)->getName(), (*it)->connection()->getDataSource() );
                    Logger::log() <<Logger::Info << "Reporting port " << (*it)->getName()<<" : ok."<<Logger::endl;
                } else {
                    Logger::log() <<Logger::Warning << "Could not report port " << (*it)->getName()<<" : not connected."<<Logger::endl;
                }
            }
            return true;
        }

            
        bool unreportComponent( const std::string& component ) {
            ORO_Execution::TaskContext* comp = this->getPeer(component);
            using namespace ORO_CoreLib;
            if ( !comp ) {
                Logger::log() <<Logger::Error << "Could not unreport Component " << component <<" : no such peer."<<Logger::endl;
                return false;
            }
            Ports ports   = comp->ports()->getPorts();
            for (Ports::iterator it = ports.begin(); it != ports.end() ; ++it) {
                this->unreportData( component + "." + (*it)->getName() );
            }
        }

        // report a specific connection.
        bool reportConnection(const std::string& component, const std::string& port ) {
            ORO_Execution::TaskContext* comp = this->getPeer(component);
            using namespace ORO_CoreLib;
            using namespace ORO_Execution;
            if ( !comp ) {
                Logger::log() <<Logger::Error << "Could not report Component " << component <<" : no such peer."<<Logger::endl;
                return false;
            }
            PortInterface* porti   = comp->ports()->getPort(port);
            if ( !porti ) {
                Logger::log() <<Logger::Error << "Could not report Port " << port
                              <<" : no such port on Component "<<component<<"."<<Logger::endl;
                return false;
            }
            if ( porti->connected() ) {
                this->reportDataSource( component + "." + port, porti->connection()->getDataSource() );
                Logger::log() <<Logger::Info << "Reporting port " << port <<" : ok."<<Logger::endl;
            } else {
                Logger::log() <<Logger::Warning << "Could not report port " << port <<" : not connected."<<Logger::endl;
            }
            return true;
        }

        bool unreportConnection(const std::string& component, const std::string& port ) {
            return this->unreportData( component + "." + port );
        }

        // report a specific datasource, property,...
        bool reportData(const std::string& component,const std::string& dataname) 
        { 
            ORO_Execution::TaskContext* comp = this->getPeer(component);
            using namespace ORO_CoreLib;
            if ( !comp ) {
                Logger::log() <<Logger::Error << "Could not report Component " << component <<" : no such peer."<<Logger::endl;
                return false;
            }
            // Is it an attribute ?
            if ( comp->attributes()->getValue( dataname ) )
                return this->reportDataSource( component + "." + dataname,
                                         comp->attributes()->getValue( dataname )->getDataSource() );
            // Is it a property ?
            if ( comp->attributes()->properties() && comp->attributes()->properties()->find( dataname ) )
                return this->reportDataSource( component + "." + dataname,
                                         comp->attributes()->properties()->find( dataname )->getDataSource() );
            // Is it a datasource ?
            if ( comp->datasources()->hasMember("this",dataname) ) {
                if (comp->datasources()->getObjectFactory("this")->getArity( dataname ) == 0)
                    return this->reportDataSource( component + "." + dataname,
                                                   comp->datasources()->getObjectFactory("this")
                                                   ->create( dataname, std::vector<DataSourceBase::shared_ptr>() ) );
                Logger::log() <<Logger::Error << "Could not report Data " << dataname <<" with arity != 0."<<Logger::endl;
            }
            return false; 
        }

        bool unreportData(const std::string& component,const std::string& datasource) { 
            return this->unreportData( component +"." + datasource); 
        }

        /**
         * This real-time function makes copies of the data to be
         * reported.
         */
        void snapshot() {
            timestamp = ORO_CoreLib::TimeService::Instance()->secondsSince( starttime );
            
            // execute the copy commands (fast).
            for(Reports::iterator it = root.begin(); it != root.end(); ++it )
                (it->get<2>())->execute();
        }

        /** @} */

    protected:
        typedef std::vector<ORO_Execution::TaskContext*> ComponentList;
        ComponentList components;
        typedef std::map< std::pair<std::string,std::string>, ORO_Execution::ConnectionInterface::shared_ptr> ConnectionList;
        ConnectionList connections;
        typedef std::map< std::pair<std::string,std::string>, ORO_CoreLib::DataSourceBase::shared_ptr> DataList;
        DataList datasources;

        typedef boost::tuple<std::string,
                              ORO_CoreLib::DataSourceBase::shared_ptr,
                              boost::shared_ptr<ORO_CoreLib::CommandInterface>,
                              ORO_CoreLib::DataSourceBase::shared_ptr> DTupple;
        /**
         * Stores the 'datasource' of all reported items as properties.
         */
        typedef std::vector<DTupple> Reports;
        Reports root;

        bool reportDataSource(std::string tag, ORO_CoreLib::DataSourceBase::shared_ptr orig)
        {
            // creates a copy of the data and an update command to
            // update the copy from the original.
            ORO_CoreLib::DataSourceBase::shared_ptr clone = orig->getTypeInfo()->buildValue();
            using namespace ORO_CoreLib;
            if ( !clone ) {
                Logger::log() <<Logger::Error << "Could not report '"<< tag <<"' : unknown type." << Logger::endl;
                return false;
            }
            try {
                boost::shared_ptr<ORO_CoreLib::CommandInterface> comm( clone->updateCommand( orig.get() ) );
                assert( comm );
                root.push_back( boost::make_tuple( tag, orig, comm, clone ) );
            } catch ( bad_assignment& ba ) {
                Logger::log() <<Logger::Error << "Could not report '"<< tag <<"' : failed to create Command." << Logger::endl;
                return false;
            }
            return true;
        }

        bool unreportData(std::string tag)
        {
            for (Reports::iterator it = root.begin();
                 it != root.end(); ++it)
                if ( it->get<0>() == tag ) {
                    root.erase(it);
                    return true;
                }
            return false;
        }

        virtual bool startup() {
            if (marshallers.begin() == marshallers.end())
                this->addMarshaller( new ORO_CoreLib::TableHeaderMarshaller<std::ostream>( std::cerr ),
                                     new ORO_CoreLib::TableMarshaller<std::ostream>( std::cerr) );
            // write headers.
            this->makeReport();
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->first->serialize( report );
                it->first->flush();
            }
            this->cleanReport();
            starttime = ORO_CoreLib::TimeService::Instance()->getTicks();
        }

        void makeReport()
        {
            using namespace ORO_CoreLib;
            
            report.add( timestamp.clone() );
            for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
                DataSourceBase::shared_ptr clone = it->get<3>();
                Property<PropertyBag> subbag( it->get<0>(), "");
                if ( clone->getTypeInfo()->decomposeType( clone, subbag.value() ) )
                    report.add( subbag.clone() );
                else
                    report.add( clone->getTypeInfo()->buildProperty(it->get<0>(), "", clone) );
            }
        }

        void cleanReport()
        {
            // Only clones were added to result, so delete them.
            deletePropertyBag( report );
        }

        /**
         * This not real-time function processes the copied data.
         */
        virtual void update() {
            // Step 1: Make copies in order to 'snapshot' all data with a timestamp
            if ( autotrigger )
                this->snapshot();

            // Step 2: Prepare bag: Decompose to native types (double,int,...)
            this->makeReport();

            // Step 3: print out the result
            // write out to all marshallers
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it)
                it->second->serialize( report );

            this->cleanReport();
        }

        virtual void shutdown() {
            // tell body marshallers that serialization is done.
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->second->flush();
            }
        }

        typedef std::vector< std::pair<boost::shared_ptr<ORO_CoreLib::Marshaller>, boost::shared_ptr<ORO_CoreLib::Marshaller> > > Marshallers;
        Marshallers marshallers;
        ORO_CoreLib::PropertyBag report;
        
        ORO_CoreLib::Property<bool>   autotrigger;

        ORO_CoreLib::TimeService::ticks starttime;
        ORO_CoreLib::Property<ORO_CoreLib::TimeService::Seconds> timestamp;

    };

}

#endif
