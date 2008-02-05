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
#include <rtt/Logger.hpp>
#include <rtt/Method.hpp>
#include <rtt/Command.hpp>

// Impl.
#include <rtt/marsh/EmptyMarshaller.hpp>
#include <rtt/marsh/PropertyDemarshaller.hpp>
#include <rtt/marsh/PropertyMarshaller.hpp>
#include <fstream>

#include "ocl/ComponentLoader.hpp"

ORO_CREATE_COMPONENT_TYPE()


namespace OCL
{
  using namespace std;
  using namespace RTT;

  ReportingComponent::ReportingComponent( std::string name /*= "Reporting" */ ) 
        : TaskContext( name ),
          report("Report"),
          autotrigger("AutoTrigger","When set to 1, the data is taken upon each update(), "
                      "otherwise, the data is only taken when the user invokes 'snapshot()'.",
                      true),
          config("Configuration","The name of the property file which lists what is to be reported.",
                 "cpf/reporter.cpf"),
          writeHeader("WriteHeader","Set to true to start each report with a header.", true),
          starttime(0), 
          timestamp("TimeStamp","The time at which the data was read.",0.0)
    {
        this->properties()->addProperty( &autotrigger );
        this->properties()->addProperty( &config );
        this->properties()->addProperty( &writeHeader );

        // Add the methods, methods make sure that they are 
        // executed in the context of the (non realtime) caller.

        this->methods()->addMethod( method( "snapshot", &ReportingComponent::snapshot , this),
                    "Take a new shapshot of the data and set the timestamp." );
        this->methods()->addMethod( method( "load", &ReportingComponent::load , this),
                    "Read the Configuration file, equivalent to configure()." );
        this->methods()->addMethod( method( "store", &ReportingComponent::store , this),
                    "Write the Configuration file, equivalent to cleanup()." );
        this->methods()->addMethod( method( "screenComponent", &ReportingComponent::screenComponent , this),
                    "Display the variables and ports of a Component.",
                    "Component", "Name of the Component" );
        this->methods()->addMethod( method( "reportComponent", &ReportingComponent::reportComponent , this),
                    "Add a Component for reporting. Only works if Component is connected.",
                    "Component", "Name of the Component" );
        this->methods()->addMethod( method( "unreportComponent", &ReportingComponent::unreportComponent , this),
                    "Remove a Component from reporting.",
                    "Component", "Name of the Component"
                     );
        this->methods()->addMethod( method( "reportData", &ReportingComponent::reportData , this),
                    "Add a Component's DataSource for reporting. Only works if DataObject exists and Component is connected.",
                    "Component", "Name of the Component",
                    "DataObject", "Name of the DataObject. For example, a property or attribute." );
        this->methods()->addMethod( method( "unreportData", &ReportingComponent::unreportData , this),
                    "Remove a DataObject from reporting.",
                    "Component", "Name of the Component",
                    "DataObject", "Name of the DataObject." );
        this->methods()->addMethod( method( "reportPort", &ReportingComponent::reportPort , this),
                    "Add a Component's Connection or Port for reporting.",
                    "Component", "Name of the Component",
                    "Port", "Name of the Port to the connection." );
        this->methods()->addMethod( method( "unreportPort", &ReportingComponent::unreportPort , this),
                    "Remove a Connection for reporting.",
                    "Component", "Name of the Component",
                    "Port", "Name of the Port to the connection." );

    }

    ReportingComponent::~ReportingComponent() {}


    bool ReportingComponent::addMarshaller( Marshaller* headerM, Marshaller* bodyM)
    {
        boost::shared_ptr<Marshaller> header(headerM);
        boost::shared_ptr<Marshaller> body(bodyM);
        if ( !header && !body)
            return false;
        if ( !header )
            header.reset( new EmptyMarshaller() );
        if ( !body)
            body.reset( new EmptyMarshaller());

        marshallers.push_back( std::make_pair( header, body ) );
        return true;
    }

    bool ReportingComponent::removeMarshallers()
    {
        marshallers.clear();
        return true;
    }


    bool ReportingComponent::store()
    {
        Logger::In in("ReportingComponent");
        log(Info) << "Writing Ports to report to file." <<endlog();
        PropertyMarshaller marsh( config.get() );
        PropertyBag bag;
        Reports::iterator it = root.begin();
        while ( it != root.end() ) {
            string tag = boost::get<0>(*it);
            string type = boost::get<4>(*it);
            bag.add( new Property<string>(type,"",tag ) );
            ++it;
        }
        marsh.serialize( bag );
        deleteProperties(bag);
        return true;
    }

    bool ReportingComponent::configureHook()
    {
        return load();
    }

    void ReportingComponent::cleanupHook()
    {
        store();
        root.clear(); // uses shared_ptr.
        deletePropertyBag( report );
    }

    bool ReportingComponent::load()
    {
        Logger::In in("ReportingComponent");
        log(Info) << "Loading Ports to report from file." <<endlog();
        PropertyDemarshaller dem( config.get() );
        PropertyBag bag;
        if (dem.deserialize( bag ) == false ) {
            log(Error) << "Reading file "<< config.get() << " failed."<<endlog();
            return false;
        }
        
        bool ok = true;
        PropertyBag::const_iterator it = bag.getProperties().begin();
        while ( it != bag.getProperties().end() )
            {
                Property<std::string>* compName = dynamic_cast<Property<std::string>* >( *it );
                if ( !compName )
                    log(Error) << "Expected Property \""
                                  << (*it)->getName() <<"\" to be of type string."<< endlog();
                else if ( compName->getName() == "Component" ) {
                    ok &= this->reportComponent( compName->value() );
                }
                else if ( compName->getName() == "Port" ) {
                    string cname = compName->value().substr(0, compName->value().find("."));
                    string pname = compName->value().substr( compName->value().find(".")+1, string::npos);
                    ok &= this->reportPort(cname, pname);
                }
                else if ( compName->getName() == "Data" ) {
                    string cname = compName->value().substr(0, compName->value().find("."));
                    string pname = compName->value().substr( compName->value().find(".")+1, string::npos);
                    ok &= this->reportData(cname, pname);
                }
                else {
                    log(Error) << "Expected \"Component\", \"Port\" or \"Data\", got "
                                  << compName->getName() << endlog();
                    ok = false;
                }
                ++it;
            }
        deleteProperties( bag );
        return ok;
    }

    bool ReportingComponent::screenComponent( const std::string& comp )
    {
        Logger::In in("ReportingComponent::screenComponent");
        log(Error) << "not implemented." <<comp<<endlog();
        return false;
    }

    bool ReportingComponent::screenImpl( const std::string& comp, std::ostream& output)
    {
        Logger::In in("ReportingComponent");
        TaskContext* c = this->getPeer(comp);
        if ( c == 0) {
            log(Error) << "Unknown Component: " <<comp<<endlog();
            return false;
        }
        output << "Screening Component '"<< comp << "' : "<< endl << endl;
        PropertyBag* bag = c->properties();
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
        Logger::In in("ReportingComponent");
        // Users may add own data sources, so avoid duplicates
        //std::vector<std::string> sources                = comp->data()->getNames();
        TaskContext* comp = this->getPeer(component);
        if ( !comp ) {
            log(Error) << "Could not report Component " << component <<" : no such peer."<<endlog();
            return false;
        }
        Ports ports   = comp->ports()->getPorts();
        for (Ports::iterator it = ports.begin(); it != ports.end() ; ++it) {
            log(Debug) << "Checking port " << (*it)->getName()<<"."<<endlog();
            this->reportPort( component, (*it)->getName() );
        }
        return true;
    }

            
    bool ReportingComponent::unreportComponent( const std::string& component ) {
        TaskContext* comp = this->getPeer(component);
        if ( !comp ) {
            log(Error) << "Could not unreport Component " << component <<" : no such peer."<<endlog();
            return false;
        }
        Ports ports   = comp->ports()->getPorts();
        for (Ports::iterator it = ports.begin(); it != ports.end() ; ++it) {
            this->unreportDataSource( component + "." + (*it)->getName() );
            if ( this->ports()->getPort( (*it)->getName() ) ) {
            }
        }
        return true;
    }

    // report a specific connection.
    bool ReportingComponent::reportPort(const std::string& component, const std::string& port ) {
        Logger::In in("ReportingComponent");
        TaskContext* comp = this->getPeer(component);
        if ( !comp ) {
            log(Error) << "Could not report Component " << component <<" : no such peer."<<endlog();
            return false;
        }
        PortInterface* porti   = comp->ports()->getPort(port);
        if ( !porti ) {
            log(Error) << "Could not report Port " << port
                          <<" : no such port on Component "<<component<<"."<<endlog();
            return false;
        }
        if ( porti->connected() ) {
            this->reportDataSource( component + "." + port, "Port", porti->connection()->getDataSource() );
            log(Info) << "Reporting port " << port <<" : ok."<<endlog();
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
            this->reportDataSource( component + "." + porti->getName(), "Port", ci->getDataSource() );
            log(Info) << "Created connection for port " << porti->getName()<<" : ok."<<endlog();
        }
        return true;
    }

    bool ReportingComponent::unreportPort(const std::string& component, const std::string& port ) {
        return this->unreportDataSource( component + "." + port );
    }

    // report a specific datasource, property,...
    bool ReportingComponent::reportData(const std::string& component,const std::string& dataname) 
    { 
        Logger::In in("ReportingComponent");
        TaskContext* comp = this->getPeer(component);
        if ( !comp ) {
            log(Error) << "Could not report Component " << component <<" : no such peer."<<endlog();
            return false;
        }
        // Is it an attribute ?
        if ( comp->attributes()->getValue( dataname ) )
            return this->reportDataSource( component + "." + dataname, "Data",
                                           comp->attributes()->getValue( dataname )->getDataSource() );
        // Is it a property ?
        if ( comp->properties() && comp->properties()->find( dataname ) )
            return this->reportDataSource( component + "." + dataname, "Data",
                                           comp->properties()->find( dataname )->getDataSource() );
        return false; 
    }

    bool ReportingComponent::unreportData(const std::string& component,const std::string& datasource) { 
        return this->unreportDataSource( component +"." + datasource); 
    }

    void ReportingComponent::snapshot() {
        timestamp = TimeService::Instance()->secondsSince( starttime );
            
        // execute the copy commands (fast).
        for(Reports::iterator it = root.begin(); it != root.end(); ++it )
            (it->get<2>())->execute();
        if( this->engine()->getActivity() )
            this->engine()->getActivity()->trigger();
    }

    bool ReportingComponent::reportDataSource(std::string tag, std::string type, DataSourceBase::shared_ptr orig)
    {
        // creates a copy of the data and an update command to
        // update the copy from the original.
        DataSourceBase::shared_ptr clone = orig->getTypeInfo()->buildValue();
        if ( !clone ) {
            log(Error) << "Could not report '"<< tag <<"' : unknown type." << endlog();
            return false;
        }
        try {
            boost::shared_ptr<CommandInterface> comm( clone->updateCommand( orig.get() ) );
            assert( comm );
            root.push_back( boost::make_tuple( tag, orig, comm, clone, type ) );
        } catch ( bad_assignment& ba ) {
            log(Error) << "Could not report '"<< tag <<"' : failed to create Command." << endlog();
            return false;
        }
        return true;
    }

    bool ReportingComponent::unreportDataSource(std::string tag)
    {
        for (Reports::iterator it = root.begin();
             it != root.end(); ++it)
            if ( it->get<0>() == tag ) {
                root.erase(it);
                return true;
            }
        return false;
    }

    bool ReportingComponent::startHook() {
        Logger::In in("ReportingComponent");
        if (marshallers.begin() == marshallers.end()) {
            log(Error) << "Need at least one marshaller to write reports." <<endlog();
            return false;
        }

        // write headers.
        if (writeHeader.get()) {
            this->snapshot();
            this->makeReport();
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->first->serialize( report );
                it->first->flush();
            }
            this->cleanReport();
        }
        starttime = TimeService::Instance()->getTicks();
        return true;
    }

    void ReportingComponent::makeReport()
    {
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

    void ReportingComponent::updateHook() {
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

    void ReportingComponent::stopHook() {
        // tell body marshallers that serialization is done.
        for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
            it->second->flush();
        }
    }

}
