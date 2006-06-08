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

#ifdef ORO_PRAGMA_INTERFACE
#pragma implementation
#endif

#include "ReportingComponent.hpp"
#include <corelib/PropertyComposition.hpp>
#include <corelib/Logger.hpp>
#include <execution/TemplateFactories.hpp>

namespace ORO_Components
{
    using namespace detail;

    ReportingComponent::ReportingComponent(std::string _name) 
        : GenericTaskContext( _name )
        splitStream(0), config(0), nh_config(0),
        reporter(0),
        period("ReportPeriod","in seconds", 0.1 ), 
        repFile("ReportFile","", "report.txt"),
        toStdOut("WriteToStdOut","", false),
        writeHeader("WriteHeader","", true)
#ifdef OROINT_OS_STDIOSTREAM
        ,toFile("WriteToFile","", false)
#endif
    {
        this->attributes()->addProperty( &period);
        this->attributes()->addProperty( &repFile);
        this->attributes()->addProperty( &toStdOut);
        this->attributes()->addProperty( &writeHeader);
#ifdef OROINT_OS_STDIOSTREAM
        this->attributes()->addProperty( &toFile);
#endif
    }

    ReportingComponent::~ReportingComponent() {
        if (serverOwner) {
            // only cleanup upon destruction, if this reportserver is still used, the application
            // will crash.
            PropertyReporter<NoHeaderMarshallTableType>::nameserver.unregisterName( reportServer );
            PropertyReporter<MarshallTableType>::nameserver.unregisterName( reportServer );
            delete reporterTask;
            delete reporter;
            delete config;
            delete nh_config;
            delete splitStream;
#ifdef OROINT_OS_STDIOSTREAM
            if ( toFile )
                {
                    delete fileStream;
                }
#endif
        }
    }

    bool ReportingComponent::startup()
    {
        Logger::In in( this->getName().c_str() );

        

        if ( writeHeader )
            {
                PropertyReporter<MarshallTableType>* tmp_ptr;
                tmp_ptr = new PropertyReporter<MarshallTableType>( *config, reportServer );
                tmp_run = tmp_ptr;
                reporter = tmp_ptr;
            }
        else
            {
                PropertyReporter<NoHeaderMarshallTableType>* tmp_ptr;
                tmp_ptr = new PropertyReporter<NoHeaderMarshallTableType>( *nh_config, reportServer );
                tmp_run = tmp_ptr;
                reporter = tmp_ptr;
            }
        return true;
    }

    void ReportingComponent::update()
    {
        // copy contents, if possible, on frequency of reporter.
        for ( DosList::iterator it = active_dos.begin(); it != active_dos.end(); ++it )
            (*it)->refresh();

        //(*it)->refreshReports( (*it)->getReports()->value() );

        if ( reporter && serverOwner )
            {
                if ( count % interval == 0 )
                    {
                        reporter->trigger(); // instruct copy.
                        count = 0;
                    }
                ++count;
            }
    }

    void ReportingComponent::finalize()
    {
        Logger::In in("Reporting");
    }

    using namespace ORO_Execution;
    MethodFactoryInterface* ReportingComponent::createMethodFactory()
    {
        // Add the methods, methods make sure that they are 
        // executed in the context of the (non realtime) caller.
        TemplateMethodFactory< ReportingComponent  >* ret =
            newMethodFactory( this );
        ret->add( "reportComponent",
                  method
                  ( &ReportingComponent::reportComponent ,
                    "Add a Component for reporting. Only works if Component exists and Kernel is not running.",
                    "Component", "Name of the Component") );
        ret->add( "reportDataObject",
                  method
                  ( &ReportingComponent::reportDataObject ,
                    "Add a DataObject for reporting. Only works if DataObject exists and Kernel is not running.",
                    "DataObject", "Name of the DataObject. For example, 'Inputs' or 'Inputs::ChannelValues'.") );
        ret->add( "unreportComponent",
                  method
                  ( &ReportingComponent::unreportComponent ,
                    "Remove a Component for reporting. Only works if Component exists and Kernel is not running.",
                    "Component", "Name of the Component") );
        ret->add( "reportDataObject",
                  method
                  ( &ReportingComponent::unreportDataObject ,
                    "Remove a DataObject for reporting. Only works if DataObject exists and Kernel is not running.",
                    "DataObject", "Name of the DataObject. For example, 'Inputs' or 'Inputs::ChannelValues'.") );
        return ret;
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
        output << "Screening Component '"<< comp << "' : "<< nl << nl;
        PropertyBag* bag = c->properties();
        if (bag) {
            output << "Properties :" << nl;
            for (PropertyBag::iterator it= bag->begin(); it != bag->end(); ++it)
                output << "  " << (*it)->getName() << " : " << *(*it)->getDataSource() << nl;
        }
        AttributeRepository::AttributeNames atts = c->attributes()->names();
        if ( !atts.empty() ) {
            output << "Attributes :" << nl;
            for (AttributeRepository::AttributeNames::iterator it= atts.begin(); it != atts.end(); ++it)
                output << "  " << *it << " : " << *c->attributes()->getValue(*it)->getDataSource() << nl;
        }
    }

    // report the datasources.
    bool ReportingComponent::reportComponent( const std::string& comp );
    bool ReportingComponent::unreportComponent( const std::string& comp );
    // report a specific connection.
    bool ReportingComponent::reportConnection(const std::string& comp, const std::string& port );
    bool ReportingComponent::unreportConnection(const std::string& comp, const std::string& port );
    // report a specific datasource.
    bool ReportingComponent::reportDataSource(const std::string& comp,const std::string& datasource);
    bool ReportingComponent::unreportDataSource(const std::string& comp,const std::string& datasource);

}
