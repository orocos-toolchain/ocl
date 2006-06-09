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

#include <corelib/RTT.hpp>

namespace Orocos
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
        : public RTT::GenericTaskContext
    {
        bool screenImpl( const std::string& comp, std::ostream& output);
    public:

        typedef RTT::DataFlowInterface::Ports Ports;

        /**
         * Set up a component for reporting.
         */
        ReportingComponent( std::string name = "Reporting" );

        virtual ~ReportingComponent();

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
        bool addMarshaller( RTT::Marshaller* headerM, RTT::Marshaller* bodyM);

        /**
         * @name Script Methods
         * @{
         */
        // write interface to file+values of prop/attrs.
        bool screenComponent( const std::string& comp );
        bool screenComponentToFile( const std::string& comp, const std::string& filename );

        // report the datasources.
        bool reportComponent( const std::string& component );
            
        bool unreportComponent( const std::string& component );

        // report a specific connection.
        bool reportPort(const std::string& component, const std::string& port );

        bool unreportPort(const std::string& component, const std::string& port );

        // report a specific datasource, property,...
        bool reportData(const std::string& component,const std::string& dataname);

        bool unreportData(const std::string& component,const std::string& datasource);

        /**
         * This real-time function makes copies of the data to be
         * reported.
         */
        void snapshot();

        /** @} */

    protected:
        typedef std::vector<RTT::TaskContext*> ComponentList;
        ComponentList components;
        typedef std::map< std::pair<std::string,std::string>, RTT::ConnectionInterface::shared_ptr> ConnectionList;
        ConnectionList connections;
        typedef std::map< std::pair<std::string,std::string>, RTT::DataSourceBase::shared_ptr> DataList;
        DataList datasources;

        typedef boost::tuple<std::string,
                              RTT::DataSourceBase::shared_ptr,
                              boost::shared_ptr<RTT::CommandInterface>,
                              RTT::DataSourceBase::shared_ptr> DTupple;
        /**
         * Stores the 'datasource' of all reported items as properties.
         */
        typedef std::vector<DTupple> Reports;
        Reports root;

        bool reportDataSource(std::string tag, RTT::DataSourceBase::shared_ptr orig);

        bool unreportData(std::string tag);

        virtual bool startup();

        void makeReport();

        void cleanReport();

        /**
         * This not real-time function processes the copied data.
         */
        virtual void update();

        virtual void shutdown();

        typedef std::vector< std::pair<boost::shared_ptr<RTT::Marshaller>, boost::shared_ptr<RTT::Marshaller> > > Marshallers;
        Marshallers marshallers;
        RTT::PropertyBag report;
        
        RTT::Property<bool>   autotrigger;

        RTT::TimeService::ticks starttime;
        RTT::Property<RTT::TimeService::Seconds> timestamp;

    };

}

#endif
