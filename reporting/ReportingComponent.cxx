/***************************************************************************
  tag: Peter Soetens  Mon May 10 19:10:38 CEST 2004  ReportingComponent.cxx 

                        ReportingComponent.cxx -  description
                           -------------------
    begin                : Mon May 10 2004
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

#include "ReportingComponent.hpp"
#include <corelib/Logger.hpp>

// Impl.
#include <execution/TemplateFactories.hpp>
#include <corelib/marshalling/TableMarshaller.hpp>
#include <corelib/marshalling/TableHeaderMarshaller.hpp>
#include <corelib/marshalling/EmptyMarshaller.hpp>
#include <iostream>
#include <fstream>




namespace Orocos
{
    using namespace RTT;
    using namespace std;

    ReportingComponent::ReportingComponent( std::string name /*= "Reporting" */ ) 
        : GenericTaskContext( name ),
          report("Report"),
          autotrigger("AutoTrigger","When set to 1, the data is taken upon each update, "
                      "otherwise, the data is only taken when the user invokes 'update()'.",
                      true),
          starttime(0), timestamp("TimeStamp","The time at which the data was read.",0.0)
    {
        this->attributes()->addProperty( &autotrigger );
        this->attributes()->addProperty( &autotrigger );

        // Add the methods, methods make sure that they are 
        // executed in the context of the (non realtime) caller.
        TemplateMethodFactory< ReportingComponent  >* ret =
            newMethodFactory( this );
        ret->add( "snapshot",
                  method
                  ( &ReportingComponent::snapshot ,
                    "Take a new shapshot of the data and set the timestamp.") );
        ret->add( "screenComponent",
                  method
                  ( &ReportingComponent::screenComponent ,
                    "Display the variables and ports of a Component.",
                    "Component", "Name of the Component") );
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
        ret->add( "reportPort",
                  method
                  ( &ReportingComponent::reportPort ,
                    "Add a Component's Connection or Port for reporting.",
                    "Component", "Name of the Component",
                    "Port", "Name of the Port to the connection.") );
        ret->add( "unreportPort",
                  method
                  ( &ReportingComponent::unreportPort ,
                    "Remove a Connection for reporting.",
                    "Component", "Name of the Component",
                    "Port", "Name of the Port to the connection.") );
        this->methods()->registerObject("this", ret);
    }

    ReportingComponent::~ReportingComponent() {}

    bool ReportingComponent::addMarshaller( RTT::Marshaller* headerM, RTT::Marshaller* bodyM)
    {
        boost::shared_ptr<RTT::Marshaller> header(headerM);
        boost::shared_ptr<RTT::Marshaller> body(bodyM);
        if ( !header && !body)
            return false;
        if ( !header )
            header.reset( new RTT::EmptyMarshaller() );
        if ( !body)
            body.reset( new RTT::EmptyMarshaller());

        marshallers.push_back( std::make_pair( header, body ) );
        return true;
    }

    bool ReportingComponent::screenComponent( const std::string& comp )
    {
        Logger::In in("screenComponent");
        if (!cout)
            return false;
        return this->screenImpl( comp, cout );
    }

    bool ReportingComponent::screenComponentToFile( const std::string& comp, const std::string& filename )
    {
        Logger::In in("screenComponentToFile");
        ofstream file( filename.c_str() );
        if (!file)
            return false;
        return this->screenImpl( comp, file );
    }

    bool ReportingComponent::screenImpl( const std::string& comp, std::ostream& output)
    {
        TaskContext* c = this->getPeer(comp);
        if ( c == 0) {
            Logger::log() <<Logger::Error << "Unknown Component: " <<comp<<Logger::endl;
            return false;
        }
        output << "Screening Component '"<< comp << "' : "<< endl << endl;
        PropertyBag* bag = c->attributes()->properties();
        if (bag) {
            output << "Properties :" << endl;
            for (PropertyBag::iterator it= bag->begin(); it != bag->end(); ++it)
                output << "  " << (*it)->getName() << " : " << (*it)->getDataSource() << endl;
        }
        AttributeRepository::AttributeNames atts = c->attributes()->names();
        if ( !atts.empty() ) {
            output << "Attributes :" << endl;
            for (AttributeRepository::AttributeNames::iterator it= atts.begin(); it != atts.end(); ++it)
                output << "  " << *it << " : " << c->attributes()->getValue(*it)->getDataSource() << endl;
        }

        vector<string> ports = c->ports()->getPortNames();
        if ( !ports.empty() ) {
            output << "Ports :" << endl;
            for (vector<string>::iterator it= ports.begin(); it != ports.end(); ++it) {
                output << "  " << *it << " : "; 
                if (c->ports()->getPort(*it)->connected() )
                    output << c->ports()->getPort(*it)->connection()->getDataSource() << endl;
                else
                    output << "(not connected)" << endl;
            }
        }
        return true;
    }

    bool ReportingComponent::reportComponent( const std::string& component ) { 
        // Users may add own data sources, so avoid duplicates
        //std::vector<std::string> sources                = comp->data()->getNames();
        RTT::TaskContext* comp = this->getPeer(component);
        using namespace ORO_CoreLib;
        if ( !comp ) {
            Logger::log() <<Logger::Error << "Could not report Component " << component <<" : no such peer."<<Logger::endl;
            return false;
        }
        Ports ports   = comp->ports()->getPorts();
        for (Ports::iterator it = ports.begin(); it != ports.end() ; ++it) {
            Logger::log() <<Logger::Debug << "Checking port " << (*it)->getName()<<"."<<Logger::endl;
            this->reportPort( component, (*it)->getName() );
        }
        return true;
    }

            
    bool ReportingComponent::unreportComponent( const std::string& component ) {
        RTT::TaskContext* comp = this->getPeer(component);
        using namespace ORO_CoreLib;
        if ( !comp ) {
            Logger::log() <<Logger::Error << "Could not unreport Component " << component <<" : no such peer."<<Logger::endl;
            return false;
        }
        Ports ports   = comp->ports()->getPorts();
        for (Ports::iterator it = ports.begin(); it != ports.end() ; ++it) {
            this->unreportData( component + "." + (*it)->getName() );
            if ( this->ports()->getPort( (*it)->getName() ) ) {
            }
        }
    }

    // report a specific connection.
    bool ReportingComponent::reportPort(const std::string& component, const std::string& port ) {
        RTT::TaskContext* comp = this->getPeer(component);
        using namespace ORO_CoreLib;
        using namespace RTT;
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
            // create new port temporarily 
            // this port is only created with the purpose of
            // creating a connection object.
            PortInterface* ourport = porti->antiClone();
            assert(ourport);

            ConnectionInterface::shared_ptr ci = porti->createConnection( ourport );
            if ( !ci ) 
                ci = ourport->createConnection( porti );
            if ( !ci )
                return false;
            ci->connect();
            
            delete ourport;
            this->reportDataSource( component + "." + porti->getName(), ci->getDataSource() );
            Logger::log() <<Logger::Info << "Created connection for port " << porti->getName()<<" : ok."<<Logger::endl;
        }
        return true;
    }

    bool ReportingComponent::unreportPort(const std::string& component, const std::string& port ) {
        return this->unreportData( component + "." + port );
    }

    // report a specific datasource, property,...
    bool ReportingComponent::reportData(const std::string& component,const std::string& dataname) 
    { 
        RTT::TaskContext* comp = this->getPeer(component);
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

    bool ReportingComponent::unreportData(const std::string& component,const std::string& datasource) { 
        return this->unreportData( component +"." + datasource); 
    }

    void ReportingComponent::snapshot() {
        timestamp = RTT::TimeService::Instance()->secondsSince( starttime );
            
        // execute the copy commands (fast).
        for(Reports::iterator it = root.begin(); it != root.end(); ++it )
            (it->get<2>())->execute();
    }

    bool ReportingComponent::reportDataSource(std::string tag, RTT::DataSourceBase::shared_ptr orig)
    {
        // creates a copy of the data and an update command to
        // update the copy from the original.
        RTT::DataSourceBase::shared_ptr clone = orig->getTypeInfo()->buildValue();
        using namespace ORO_CoreLib;
        if ( !clone ) {
            Logger::log() <<Logger::Error << "Could not report '"<< tag <<"' : unknown type." << Logger::endl;
            return false;
        }
        try {
            boost::shared_ptr<RTT::CommandInterface> comm( clone->updateCommand( orig.get() ) );
            assert( comm );
            root.push_back( boost::make_tuple( tag, orig, comm, clone ) );
        } catch ( bad_assignment& ba ) {
            Logger::log() <<Logger::Error << "Could not report '"<< tag <<"' : failed to create Command." << Logger::endl;
            return false;
        }
        return true;
    }

    bool ReportingComponent::unreportData(std::string tag)
    {
        for (Reports::iterator it = root.begin();
             it != root.end(); ++it)
            if ( it->get<0>() == tag ) {
                root.erase(it);
                return true;
            }
        return false;
    }

    bool ReportingComponent::startup() {
        if (marshallers.begin() == marshallers.end())
            this->addMarshaller( new RTT::TableHeaderMarshaller<std::ostream>( std::cerr ),
                                 new RTT::TableMarshaller<std::ostream>( std::cerr) );
        // write headers.
        this->makeReport();
        for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
            it->first->serialize( report );
            it->first->flush();
        }
        this->cleanReport();
        starttime = RTT::TimeService::Instance()->getTicks();
    }

    void ReportingComponent::makeReport()
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

    void ReportingComponent::cleanReport()
    {
        // Only clones were added to result, so delete them.
        deletePropertyBag( report );
    }

    void ReportingComponent::update() {
        // Step 1: Make copies in order to 'snapshot' all data with a timestamp
        if ( autotrigger )
            this->snapshot();

        // Step 2: Prepare bag: Decompose to native types (double,int,...)
        this->makeReport();

        // Step 3: print out the result
        // write out to all marshallers
        for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
            it->second->serialize( report );
            it->second->flush();
        }

        this->cleanReport();
    }

    void ReportingComponent::shutdown() {
        // tell body marshallers that serialization is done.
        for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
            it->second->flush();
        }
    }

}
