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
#include <boost/algorithm/string.hpp>

#include "ocl/Component.hpp"
#include <rtt/types/PropertyDecomposition.hpp>
#include <boost/lexical_cast.hpp>

ORO_CREATE_COMPONENT_TYPE()


namespace OCL
{
    using namespace std;
    using namespace RTT;
    using namespace RTT::detail;

    //! Helper data source to check if two sizes are *still* equal and check an upstream comparison as well.
    //! We use this to track changes in sizes for our sequences, which will lead to a rebuild.
    class CheckSizeDataSource : public ValueDataSource<bool>
    {
        mutable int msize;
        DataSource<int>::shared_ptr mds;
        DataSource<bool>::shared_ptr mupstream;
    public:
        CheckSizeDataSource(int size, DataSource<int>::shared_ptr ds, DataSource<bool>::shared_ptr upstream)
            : msize(size), mds(ds), mupstream(upstream)
        {}
        /**
         * Returns true if the size or the upstream size remained the same.
         */
        bool get() const{
            // it's very important to first check upstream, because
            // if upstream changed size, downstream might already be corrupt !
            // (downstream will be corrupt upon capacity changes upstream)
            bool result = true;
            if (mupstream)
                result = (mupstream->get() && msize == mds->get());
            else
                result = (msize == mds->get());
            msize = mds->get();
            return result;
        }
    };

    /**
     * Decompose a given type using getMember() into a property tree.
     *
     * This function shares 90% of the code with RTT::types::propertyDecomposition, but is optimised
     * for speed and only uses getMember with references, not the custom decomposeType functions.
     */
    bool memberDecomposition( base::DataSourceBase::shared_ptr dsb, PropertyBag& targetbag, DataSource<bool>::shared_ptr& resized)
    {
        assert(dsb);

        vector<string> parts = dsb->getMemberNames();
        if ( parts.empty() ) {
            return false;
        }

        targetbag.setType( dsb->getTypeName() );

        // needed for recursion.
        auto_ptr< Property<PropertyBag> > recurse_bag( new Property<PropertyBag>("recurse_bag","") );
        // First at the explicitly listed parts:
        for(vector<string>::iterator it = parts.begin(); it != parts.end(); ++it ) {
            // we first force getMember to get to the type, then we do it again but with a reference set.
            DataSourceBase::shared_ptr part = dsb->getMember( *it );
            if (!part) {
                log(Error) <<"memberDecomposition: Inconsistent type info for "<< dsb->getTypeName() << ": reported to have part '"<<*it<<"' but failed to return it."<<endlog();
                continue;
            }
            if ( !part->isAssignable() ) {
                // For example: the case for size() and capacity() in SequenceTypeInfo
                log(Debug)<<"memberDecomposition: Part "<< *it << ":"<< part->getTypeName() << " is not changeable."<<endlog();
                continue;
            }
            // now the reference magic:
            DataSourceBase::shared_ptr ref = part->getTypeInfo()->buildReference( 0 );
            dsb->getTypeInfo()->getMember( dynamic_cast<Reference*>(ref.get() ), dsb, *it); // fills in ref
            // newpb will contain a reference to the port's datasource data !
            PropertyBase* newpb = part->getTypeInfo()->buildProperty(*it,"Part", ref);
            if ( !newpb ) {
                log(Error)<< "Decomposition failed because Part '"<<*it<<"' is not known to type system."<<endlog();
                continue;
            }
            // finally recurse or add it to the target bag:
            if ( !memberDecomposition( ref, recurse_bag->value(), resized) ) {
                assert( recurse_bag->value().empty() );
                // finally: check for conversions (enums use this):
                base::DataSourceBase::shared_ptr converted = newpb->getTypeInfo()->convertType( dsb );
                if ( converted && converted != dsb ) {
                    // converted contains another type.
                    targetbag.add( converted->getTypeInfo()->buildProperty(*it, "", converted) );
                    delete newpb;
                } else
                    targetbag.ownProperty( newpb ); // leaf
            } else {
                recurse_bag->setName(*it);
                // setType() is done by recursive of self.
                targetbag.ownProperty( recurse_bag.release() ); //recursed.
                recurse_bag.reset( new Property<PropertyBag>("recurse_bag","") );
                delete newpb; // since we recursed, the recurse_bag now 'embodies' newpb.
            }
        }

        // Next get the numbered parts. This is much more involved since sequences may be resizable.
        // We keep track of the size, and if that changes, we will have to force a re-decomposition
        // of the sequence's internals.
        DataSource<int>::shared_ptr size = DataSource<int>::narrow( dsb->getMember("size").get() );
        if (size) {
            int msize = size->get();
            for (int i=0; i < msize; ++i) {
                string indx = boost::lexical_cast<string>( i );
                DataSourceBase::shared_ptr item = dsb->getMember(indx);
                resized = new CheckSizeDataSource( msize, size, resized );
                if (item) {
                    if ( !item->isAssignable() ) {
                        // For example: the case for size() and capacity() in SequenceTypeInfo
                        log(Warning)<<"memberDecomposition: Item '"<< indx << "' of type "<< dsb->getTypeName() << " is not changeable."<<endlog();
                        continue;
                    }
                    // finally recurse or add it to the target bag:
                    PropertyBase* newpb = item->getTypeInfo()->buildProperty( indx,"",item);
                    if ( !memberDecomposition( item, recurse_bag->value(), resized) ) {
                        targetbag.ownProperty( newpb ); // leaf
                    } else {
                        delete newpb;
                        recurse_bag->setName( indx );
                        // setType() is done by recursive of self.
                        targetbag.ownProperty( recurse_bag.release() ); //recursed.
                        recurse_bag.reset( new Property<PropertyBag>("recurse_bag","") );
                    }
                }
            }
        }
        if (targetbag.empty() )
            log(Debug) << "memberDecomposition: "<<  dsb->getTypeName() << " returns an empty property bag." << endlog();
        return true;
    }

  ReportingComponent::ReportingComponent( std::string name /*= "Reporting" */ )
        : TaskContext( name ),
          report("Report"), snapshotted(false),
          writeHeader("WriteHeader","Set to true to start each report with a header.", true),
          decompose("Decompose","Set to false in order to not decompose the port data. The marshaller must be able to handle this itself for this to work.", true),
          insnapshot("Snapshot","Set to true to enable snapshot mode. This will cause a non-periodic reporter to only report data upon the snapshot() operation.",false),
          synchronize_with_logging("Synchronize","Set to true if the timestamp should be synchronized with the logging",false),
          report_data("ReportData","A PropertyBag which defines which ports or components to report."),
          report_policy( ConnPolicy::data(ConnPolicy::LOCK_FREE,true,false) ),
          onlyNewData(false),
          starttime(0),
          timestamp("TimeStamp","The time at which the data was read.",0.0)
    {
        this->provides()->doc("Captures data on data ports. A periodic reporter will sample each added port according to its period, a non-periodic reporter will write out data as it comes in, or only during a snapshot() if the Snapshot property is true.");

        this->properties()->addProperty( writeHeader );
        this->properties()->addProperty( decompose );
        this->properties()->addProperty( insnapshot );
        this->properties()->addProperty( synchronize_with_logging);
        this->properties()->addProperty( report_data);
        this->properties()->addProperty( "ReportPolicy", report_policy).doc("The ConnPolicy for the reporter's port connections.");
        this->properties()->addProperty( "ReportOnlyNewData", onlyNewData).doc("Turn on in order to only write out NewData on ports and omit unchanged ports. Turn off in order to sample and write out all ports (even old data).");
        // Add the methods, methods make sure that they are
        // executed in the context of the (non realtime) caller.

        this->addOperation("snapshot", &ReportingComponent::snapshot , this, RTT::OwnThread).doc("Take a new shapshot of all data and cause them to be written out.");
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
        std::vector<std::string> strs;
        boost::split(strs, port, boost::is_any_of("."));

        // strs could be empty because of a bug in Boost 1.44 (see https://svn.boost.org/trac/boost/ticket/4751)
        if (strs.empty()) return false;

        Service::shared_ptr service=comp->provides();
        while ( strs.size() != 1 && service) {
            service = service->getService( strs.front() );
            if (service)
                strs.erase( strs.begin() );
        }
        if (!service) {
            log(Error) <<"No such service: '"<< strs.front() <<"' while looking for port '"<< port<<"'"<<endlog();
            return 0;
        }
        base::PortInterface* porti = 0;
        porti = service->getPort(strs.front());
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
        ourport->setName(component + "_" + port);
        ipi = dynamic_cast<base::InputPortInterface*> (ourport);
        assert(ipi);

        if (report_policy.type == ConnPolicy::DATA ) {
            log(Info) << "Not buffering of data flow connections. You may miss samples." <<endlog();
        } else {
            log(Info) << "Buffering ports with size "<< report_policy.size << ", as set in ReportPolicy property." <<endlog();
        }

        this->ports()->addEventPort( *ipi );
        if (porti->connectTo(ourport, report_policy ) == false)
        {
            log(Error) << "Could not connect to OutputPort " << porti->getName() << endlog();
            this->ports()->removePort(ourport->getName());
            delete ourport; // XXX/TODO We're leaking ourport !
            return false;
        }

        if (this->reportDataSource(component + "." + port, "Port",
                                   ipi->getDataSource(),ipi, true) == false)
        {
            log(Error) << "Failed reporting port " << port << endlog();
            this->ports()->removePort(ourport->getName());
            delete ourport;
            return false;
        }

        log(Info) << "Monitoring OutputPort " << port << " : ok." << endlog();
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
                                        comp->provides()->getValue( dataname )->getDataSource(), 0,  false ) == false) {
                log(Error) << "Failed reporting data " << dataname <<endlog();
                return false;
            }
        }

        // Is it a property ?
        if ( comp->properties() && comp->properties()->find( dataname ) ) {
            if (this->reportDataSource( component + "." + dataname, "Data",
                                        comp->properties()->find( dataname )->getDataSource(), 0, false ) == false) {
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

    bool ReportingComponent::reportDataSource(std::string tag, std::string type, base::DataSourceBase::shared_ptr orig, base::InputPortInterface* ipi, bool track)
    {
        // check for duplicates:
        for (Reports::iterator it = root.begin();
             it != root.end(); ++it)
            if ( it->get<T_QualName>() == tag ) {
                return true;
            }

        // creates a copy of the data and an update command to
        // update the copy from the original.
        base::DataSourceBase::shared_ptr clone = orig->getTypeInfo()->buildValue();
        if ( !clone ) {
            log(Error) << "Could not report '"<< tag <<"' : unknown type." << endlog();
            return false;
        }
        PropertyBase* prop = 0;
        root.push_back( boost::make_tuple( tag, orig, type, prop, ipi, false, track ) );
        return true;
    }

    bool ReportingComponent::unreportDataSource(std::string tag)
    {
        for (Reports::iterator it = root.begin();
             it != root.end(); ++it)
            if ( it->get<T_QualName>() == tag ) {
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

        if(synchronize_with_logging.get())
            starttime = Logger::Instance()->getReferenceTime();
        else
            starttime = os::TimeService::Instance()->getTicks();

        // Get initial data samples
        this->copydata();
        this->makeReport2();

        // write headers
        if (writeHeader.get()) {
            // call all header marshallers.
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->first->serialize( report );
                it->first->flush();
            }
        }

        // write initial values with all value marshallers (uses the forcing above)
        if ( getActivity()->isPeriodic() ) {
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                it->second->serialize( report );
                it->second->flush();
            }
        }

        // Turn off port triggering in snapshot mode, and vice versa.
        // Also clears any old data in the buffers
        for(Reports::iterator it = root.begin(); it != root.end(); ++it )
            if ( it->get<T_Port>() ) {
#ifndef ORO_SIGNALLING_PORTS
                it->get<T_Port>()->signalInterface( !insnapshot.get() );
#endif
                it->get<T_Port>()->clear();
            }


        snapshotted = false;
        return true;
    }

    void ReportingComponent::snapshot() {
        // this function always copies and reports all data It's run in ownthread, so updateHook will be run later.
        if ( getActivity()->isPeriodic() )
            return;
        snapshotted = true;
        updateHook();
    }

    bool ReportingComponent::copydata() {
        timestamp = os::TimeService::Instance()->secondsSince( starttime );

        // result will become true if more data is to be read.
        bool result = false;
        // This evaluates the InputPortDataSource evaluate() returns true upon new data.
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            it->get<T_NewData>() = (it->get<T_PortDS>())->evaluate(); // stores 'NewData' flag.
            // if its a property/attr, get<T_NewData> will always be true, so we override (clear) with get<T_Tracked>.
            result = result || ( it->get<T_NewData>() && it->get<T_Tracked>() );
        }
        return result;
    }

    void ReportingComponent::makeReport2()
    {
        // Uses the port DS itself to make the report.
        assert( report.empty() );
        // For the timestamp, we need to add a new property object:
        report.add( timestamp.getTypeInfo()->buildProperty( timestamp.getName(), "", timestamp.getDataSource() ) );
        DataSource<bool>::shared_ptr checker;
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            Property<PropertyBag>* subbag = new Property<PropertyBag>( it->get<T_QualName>(), "");
            if ( decompose.get() && memberDecomposition( it->get<T_PortDS>(), subbag->value(), checker ) ) {
                report.add( subbag );
                it->get<T_Property>() = subbag;
            } else {
                // property or simple value port...
                base::DataSourceBase::shared_ptr converted = it->get<T_PortDS>()->getTypeInfo()->convertType( it->get<T_PortDS>() );
                if ( converted && converted != it->get<T_PortDS>() ) {
                    // converted contains another type.
                    PropertyBase* convProp = converted->getTypeInfo()->buildProperty(it->get<T_QualName>(), "", converted);
                    it->get<T_Property>() = convProp;
                    report.add(convProp);
                } else {
                    PropertyBase* origProp = it->get<T_PortDS>()->getTypeInfo()->buildProperty(it->get<T_QualName>(), "", it->get<T_PortDS>());
                    it->get<T_Property>() = origProp;
                    report.add(origProp);
                }
                delete subbag;
            }

        }
        mchecker = checker;
    }
        
    void ReportingComponent::cleanReport()
    {
        // Only clones were added to result, so delete them.
        deletePropertyBag( report );
    }

    void ReportingComponent::updateHook() {
        //If not periodic and insnapshot is true, only continue if snapshot is called.
        if( !getActivity()->isPeriodic() && insnapshot.get() && !snapshotted)
            return;
        else
            snapshotted = false;

        // if any data sequence got resized, we rebuild the whole bunch.
        // otherwise, we need to track every individual array (not impossible though, but still needs an upstream concept).
        if ( mchecker && mchecker->get() == false ) {
            cleanReport();
            makeReport2();
        } else
            copydata();

        do {
            // Step 3: print out the result
            // write out to all marshallers
            for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
                if ( onlyNewData ) {
                    // Serialize only changed ports:
                    it->second->serialize( *report.begin() ); // TimeStamp.
                    for (Reports::const_iterator i = root.begin();
                         i != root.end();
                         i++ )
                        {
                            if ( i->get<T_NewData>() )
                                it->second->serialize( i->get<T_Property>() );
                        }
                } else {
                    // pass on all ports to the marshaller
                    it->second->serialize( report );
                }
                it->second->flush();
            }
        } while( !getActivity()->isPeriodic() && !insnapshot.get() && copydata() ); // repeat if necessary. In periodic mode we always only sample once.
    }

    void ReportingComponent::stopHook() {
        // tell body marshallers that serialization is done.
        for(Marshallers::iterator it=marshallers.begin(); it != marshallers.end(); ++it) {
            it->second->flush();
        }
        cleanReport();
    }

}
