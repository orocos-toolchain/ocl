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

// Impl.
#include "EmptyMarshaller.hpp"
#include <rtt/marsh/PropertyDemarshaller.hpp>
#include <rtt/marsh/PropertyMarshaller.hpp>
#include <iostream>
#include <fstream>
#include <exception>

#include "ocl/Component.hpp"
#include <rtt/types/PropertyDecomposition.hpp>

ORO_CREATE_COMPONENT_TYPE()


namespace OCL
{
    using namespace std;
    using namespace RTT;
    using namespace RTT::detail;

  ReportingComponent::ReportingComponent( std::string name /*= "Reporting" */ )
        : TaskContext( name ),
          report("Report"),
          snapshotOnly("SnapshotOnly","Set to true to only log data if a snapshot() was done.", false),
          writeHeader("WriteHeader","Set to true to start each report with a header.", true),
          decompose("Decompose","Set to false in order to create multidimensional array in netcdf", true),
          synchronize_with_logging("Synchronize","Set to true if the timestamp should be synchronized with the logging",false),
          report_data("ReportData","A PropertyBag which defines which ports or components to report."),
          null("NullSample","The characters written to the log to indicate that no new data was available for that port during a snapshot(). As a special value, the string 'last' is interpreted as repeating the last value.","last"),
          starttime(0),
          timestamp("TimeStamp","The time at which the data was read.",0.0)
    {
        this->properties()->addProperty( snapshotOnly );
        this->properties()->addProperty( writeHeader );
        this->properties()->addProperty( decompose );
        this->properties()->addProperty( synchronize_with_logging);
        this->properties()->addProperty( report_data);
        this->properties()->addProperty( null);

        // Add the methods, methods make sure that they are
        // executed in the context of the (non realtime) caller.

        this->addOperation("snapshot", &ReportingComponent::snapshot , this, RTT::ClientThread).doc("Take a new shapshot of all data and cause them to be written out.");
        this->addOperation("screenComponent", &ReportingComponent::screenComponent , this, RTT::ClientThread).doc("Display the variables and ports of a Component.").arg("Component", "Name of the Component");
        this->addOperation("reportComponent", &ReportingComponent::reportComponent , this, RTT::ClientThread).doc("Add a peer Component and report all its data ports").arg("Component", "Name of the Component");
        this->addOperation("unreportComponent", &ReportingComponent::unreportComponent , this, RTT::ClientThread).doc("Remove all Component's data ports from reporting.").arg("Component", "Name of the Component");
        this->addOperation("reportData", &ReportingComponent::reportData , this, RTT::ClientThread).doc("Add a Component's Property or attribute for reporting.").arg("Component", "Name of the Component").arg("Data", "Name of the Data to report. A property's or attribute's name.");
        this->addOperation("unreportData", &ReportingComponent::unreportData , this, RTT::ClientThread).doc("Remove a Data object from reporting.").arg("Component", "Name of the Component").arg("Data", "Name of the property or attribute.");
        this->addOperation("reportPort", &ReportingComponent::reportPort , this, RTT::ClientThread).doc("Add a Component's OutputPort for reporting.").arg("Component", "Name of the Component").arg("Port", "Name of the Port.");
        this->addOperation("unreportPort", &ReportingComponent::unreportPort , this, RTT::ClientThread).doc("Remove a Port from reporting.").arg("Component", "Name of the Component").arg("Port", "Name of the Port.");

    }

    ReportingComponent::~ReportingComponent() {}


    bool ReportingComponent::addMarshaller( marsh::MarshallInterface* headerM, marsh::MarshallInterface* bodyM)
    {
        boost::shared_ptr<marsh::MarshallInterface> header(headerM);
        boost::shared_ptr<marsh::MarshallInterface> body(bodyM);
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

    void ReportingComponent::cleanupHook()
    {
        root.clear(); // uses shared_ptr.
        deletePropertyBag( report );
    }

    bool ReportingComponent::configureHook()
    {
        Logger::In in("ReportingComponent");

        // we make a copy to be allowed to iterate over and exted report_data:
        PropertyBag bag = report_data.value();

        if ( bag.empty() ) {
            log(Error) <<"No port or component configuration loaded."<<endlog();
            log(Error) <<"Please use marshalling.loadProperties(), reportComponent() (scripting) or LoadProperties (XML) in order to fill in ReportData." <<endlog();
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
                    std::string name = compName->value(); // we will delete this property !
                    this->unreportComponent( name );
                    ok &= this->reportComponent( name );
                }
                else if ( compName->getName() == "Port" ) {
                    string cname = compName->value().substr(0, compName->value().find("."));
                    string pname = compName->value().substr( compName->value().find(".")+1, string::npos);
                    if (cname.empty() || pname.empty() ) {
                        log(Error) << "The Port value '"<<compName->getName()<< "' must at least consist of a component name followed by a dot and the port name." <<endlog();
                        ok = false;
                        continue;
                    }
                    this->unreportPort(cname,pname);
                    ok &= this->reportPort(cname, pname);
                }
                else if ( compName->getName() == "Data" ) {
                    string cname = compName->value().substr(0, compName->value().find("."));
                    string pname = compName->value().substr( compName->value().find(".")+1, string::npos);
                    if (cname.empty() || pname.empty() ) {
                        log(Error) << "The Data value '"<<compName->getName()<< "' must at least consist of a component name followed by a dot and the property/attribute name." <<endlog();
                        ok = false;
                        continue;
                    }
                    this->unreportData(cname,pname);
                    ok &= this->reportData(cname, pname);
                }
                else {
                    log(Error) << "Expected \"Component\", \"Port\" or \"Data\", got "
                                  << compName->getName() << endlog();
                    ok = false;
                }
                ++it;
            }
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
        ConfigurationInterface::AttributeNames atts = c->provides()->getAttributeNames();
        if ( !atts.empty() ) {
            output << "Attributes :" << endl;
            for (ConfigurationInterface::AttributeNames::iterator it= atts.begin(); it != atts.end(); ++it)
                output << "  " << *it << " : " << c->provides()->getValue(*it)->getDataSource() << endl;
        }

        vector<string> ports = c->ports()->getPortNames();
        if ( !ports.empty() ) {
            output << "Ports :" << endl;
            for (vector<string>::iterator it= ports.begin(); it != ports.end(); ++it) {
                output << "  " << *it << " : ";
                if (c->ports()->getPort(*it)->connected() )
                    output << "(connected)" << endl;
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
        if ( !report_data.value().findValue<string>(component) )
            report_data.value().ownProperty( new Property<string>("Component","",component) );
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
            unreportPort(component, (*it)->getName() );
        }
        base::PropertyBase* pb = report_data.value().findValue<string>(component);
        if (pb)
            report_data.value().removeProperty( pb );// pb is deleted by bag
        return true;
    }

    // report a specific connection.
    bool ReportingComponent::reportPort(const std::string& component, const std::string& port ) {
        Logger::In in("ReportingComponent");
        TaskContext* comp = this->getPeer(component);
        if ( this->ports()->getPort(component +"_"+port) ) {
            log(Warning) <<"Already reporting "<<component<<"."<<port<<": removing old port first."<<endlog();
            this->unreportPort(component,port);
        }
        if ( !comp ) {
            log(Error) << "Could not report Component " << component <<" : no such peer."<<endlog();
            return false;
        }
        base::PortInterface* porti   = comp->ports()->getPort(port);
        if ( !porti ) {
            log(Error) << "Could not report Port " << port
                          <<" : no such port on Component "<<component<<"."<<endlog();
            return false;
        }

        base::InputPortInterface* ipi =  dynamic_cast<base::InputPortInterface*>(porti);
        if (ipi) {
            log(Error) << "Can not report InputPort "<< porti->getName() <<" of Component " << component <<endlog();
            return false;
        }
            // create new port temporarily
        // this port is only created with the purpose of
        // creating a connection object.
        base::PortInterface* ourport = porti->antiClone();
        assert(ourport);
        ourport->setName(component + "_" + porti->getName());
        ipi = dynamic_cast<base::InputPortInterface*> (ourport);
        assert(ipi);

        ConnPolicy pol;
        if (snapshotOnly.get() ) {
            log(Info) << "Disabling buffering of data flow connections in SnapshotOnly mode." <<endlog();
            pol = ConnPolicy::data(ConnPolicy::LOCK_FREE,true,false);
        } else {
            log(Info) << "Buffering of data flow connections is set to 10 samples." <<endlog();
            pol = ConnPolicy::buffer(10,ConnPolicy::LOCK_FREE,true,false);
        }

        if (porti->connectTo(ourport, ConnPolicy::buffer(10,ConnPolicy::LOCK_FREE,true,false) ) == false)
        {
            log(Error) << "Could not connect to OutputPort " << porti->getName() << endlog();
            delete ourport; // XXX/TODO We're leaking ourport !
            return false;
        }

        if (this->reportDataSource(component + "." + porti->getName(), "Port",
                                   ipi->getDataSource(), true) == false)
        {
            log(Error) << "Failed reporting port " << port << endlog();
            delete ourport;
            return false;
        }
        this->ports()->addEventPort( *ipi );
        log(Info) << "Monitoring OutputPort " << porti->getName() << " : ok." << endlog();
        // Add port to ReportData properties if component nor port are listed yet.
        if ( !report_data.value().findValue<string>(component) && !report_data.value().findValue<string>( component+"."+port) )
            report_data.value().ownProperty(new Property<string>("Port","",component+"."+port));
        return true;
    }

    bool ReportingComponent::unreportPort(const std::string& component, const std::string& port ) {
        base::PortInterface* ourport = this->ports()->getPort(component + "_" + port);
        if ( this->unreportDataSource( component + "." + port ) && report_data.value().removeProperty( report_data.value().findValue<string>(component+"."+port))) {
            this->ports()->removePort(ourport->getName());
            delete ourport; // also deletes datasource.
            return true;
        }
        return false;
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
        if ( comp->provides()->getValue( dataname ) ) {
            if (this->reportDataSource( component + "." + dataname, "Data",
                                        comp->provides()->getValue( dataname )->getDataSource(), false ) == false) {
                log(Error) << "Failed reporting data " << dataname <<endlog();
                return false;
            }
        }

        // Is it a property ?
        if ( comp->properties() && comp->properties()->find( dataname ) ) {
            if (this->reportDataSource( component + "." + dataname, "Data",
                                        comp->properties()->find( dataname )->getDataSource(), false ) == false) {
                log(Error) << "Failed reporting data " << dataname <<endlog();
                return false;
            }
        }
        // Ok. we passed.
        // Add port to ReportData properties if data not listed yet.
        if ( !report_data.value().findValue<string>( component+"."+dataname) )
            report_data.value().ownProperty(new Property<string>("Data","",component+"."+dataname));
        return true;
    }

    bool ReportingComponent::unreportData(const std::string& component,const std::string& datasource) {
        return this->unreportDataSource( component +"." + datasource) && report_data.value().removeProperty( report_data.value().findValue<string>(component+"."+datasource));
    }

    bool ReportingComponent::reportDataSource(std::string tag, std::string type, base::DataSourceBase::shared_ptr orig, bool track)
    {
        // check for duplicates:
        for (Reports::iterator it = root.begin();
             it != root.end(); ++it)
            if ( it->get<0>() == tag ) {
                return true;
            }

        // creates a copy of the data and an update command to
        // update the copy from the original.
        base::DataSourceBase::shared_ptr clone = orig->getTypeInfo()->buildValue();
        if ( !clone ) {
            log(Error) << "Could not report '"<< tag <<"' : unknown type." << endlog();
            return false;
        }
        try {
            boost::shared_ptr<base::ActionInterface> comm( clone->updateAction( orig.get() ) );
            assert( comm );
            root.push_back( boost::make_tuple( tag, orig, comm, clone, type, false, track ) );
        } catch ( internal::bad_assignment& ba ) {
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

        // Write initial lines.
        this->copydata();
        // force all as new data:
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            it->get<5>() = true;
        }
        this->makeReport();

        // write headers
        if (writeHeader.get()) {
            // call all header marshallers.
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->first->serialize( report );
                it->first->flush();
            }
        }

        // write initial values with all value marshallers.
        if (snapshotOnly.get() == false) {
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->second->serialize( report );
                it->second->flush();
            }
        }

        this->cleanReport();

        if(synchronize_with_logging.get())
            starttime = Logger::Instance()->getReferenceTime();
        else
            starttime = os::TimeService::Instance()->getTicks();

        return true;
    }

    void ReportingComponent::snapshot() {
        // this function always copies and reports all data
        copydata();
        // force logging of all data.
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            it->get<5>() = true;
        }

        if( this->engine()->getActivity() )
            this->engine()->getActivity()->trigger();
    }

    bool ReportingComponent::copydata() {
        timestamp = os::TimeService::Instance()->secondsSince( starttime );

        bool result = false;
        // execute the copy commands (fast).
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            bool newdata = false;
            do {
                // execute() will only return true if the InputPortSource's evaluate()
                // returned true too during readArguments().
                (it->get<2>())->readArguments();
                newdata = (it->get<2>())->execute(); // stores new data flag.
                (it->get<5>()) |= newdata;
                // if its a property/attr, get<5> will always be true, so we override with get<6>.
                result |= newdata && (it->get<6>());
                // if periodic, keep reading until we hit the last sample (or none are available):
            } while ( newdata && (it->get<6>()) && this->getActivity()->isPeriodic() );
        }
        return result;
    }

    void ReportingComponent::makeReport()
    {
        // This function must clone/copy all data samples because we apply
        // decomposition of structs. So at the end of this function there are
        // three instances of each data sample:
        // 1. the one in the input port's datasource
        // 2. the 1:1 copy made during copydata(), present in 'root'.
        // 3. the decomposition of the copy, present in 'report'.
        report.add( timestamp.clone() );
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            if (  it->get<5>() || null.rvalue() == "last" ) {
                base::DataSourceBase::shared_ptr clone = it->get<3>();
                Property<PropertyBag>* subbag = new Property<PropertyBag>( it->get<0>(), "");
                if ( decompose.get() && typeDecomposition( clone, subbag->value() ) )
                    report.add( subbag );
                else {
                    base::DataSourceBase::shared_ptr converted = clone->getTypeInfo()->decomposeType(clone);
                    if ( converted && converted != clone ) {
                        // converted contains another type, or a property bag.
                        report.add( converted->getTypeInfo()->buildProperty(it->get<0>(), "", converted) );
                    } else
                        // use the original clone.
                        report.add( clone->getTypeInfo()->buildProperty(it->get<0>(), "", clone) );
                    delete subbag;
                }
                it->get<5>() = false;
            } else {
                //  no new data
                report.add( null.clone() );
            }
        }
        timestamp = 0.0; // reset.
    }

    void ReportingComponent::cleanReport()
    {
        // Only clones were added to result, so delete them.
        deletePropertyBag( report );
    }

    void ReportingComponent::updateHook() {
        // in snapshot only mode we only log if data has been copied by snapshot()
        if (snapshotOnly.get() && timestamp == 0.0)
            return;
        // Step 1: Make copies in order to copy all data and get the timestamp
        //
        if ( !snapshotOnly.get() )
            this->copydata();

        do {
            // Step 2: Prepare bag: Decompose to native types (double,int,...)
            this->makeReport();

            // Step 3: print out the result
            // write out to all marshallers
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->second->serialize( report );
                it->second->flush();
            }

            this->cleanReport();
        } while( copydata() && !snapshotOnly.get() ); // repeat if necessary and not in snapshotting.
    }

    void ReportingComponent::stopHook() {
        // tell body marshallers that serialization is done.
        for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
            it->second->flush();
        }
    }

}
