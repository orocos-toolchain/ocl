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


#include <boost/tuple/tuple.hpp>

#include <rtt/Property.hpp>
#include <rtt/PropertyBag.hpp>
#include <rtt/Marshaller.hpp>
#include <rtt/TimeService.hpp>
#include <rtt/TaskContext.hpp>

#include <rtt/RTT.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @brief A Component for periodically reporting Component
     * DataObject contents to a human readable text format. The
     * default is a table with a header.
     *
     * It can report to any data format, using the 'addMarshaller'
     * function. If the user does \b not add any marshallers, the
     * properties will be used to create default marshallers.
     *
     * @par Configuration
     * The ReportingComponent (and its decendants) take two configuration
     * files. The first is the 'classical' XML '.cpf' file which contains
     * the property values of the ReportingComponent. For example, to
     * enable writing a header or not. The second XML file is in the
     * same format, but describes which ports and peer components need to be
     * monitored. It has the following format:
     * @code
     <?xml version="1.0" encoding="UTF-8"?>
     <!DOCTYPE properties SYSTEM "cpf.dtd">
     <properties>
       <!-- Monitor all ports of a Component : -->
       <simple name="Component" type="string"><description></description><value>ComponentX</value></simple>

       <!-- Monitor a single Data or Buffer-Port of another Component : -->
       <simple name="Port" type="string"><description></description><value>ComponentY.PortZ</value></simple>

       <!-- add as many lines as desired... -->

     </properties>
     @endcode
     * The above file is read by the 'ReportingComponent::load()' method, the file to load
     * is listed in the 'Configuration' property.
     */
    class ReportingComponent
        : public RTT::TaskContext
    {
    protected:
        /**
         * This method writes out the status of a component's interface.
         */
        bool screenImpl( const std::string& comp, std::ostream& output);
    public:

        typedef RTT::DataFlowInterface::Ports Ports;

        /**
         * Set up a component for reporting.
         */
        ReportingComponent( std::string name = "Reporting" );

        virtual ~ReportingComponent();

        /**
         * Adds a Plugin to receive incomming data. The marshallers become
         * owned by this component.
         * @param header A marshaller which writes out a header when this
         * component is started. May be null (0).
         * @param body A marshaller wich will get periodically a serialisation
         * request to process incomming data. May be null(0).
         *
         * example:
         * addMarshaller( new HeaderMarshaller(), new ContentsMarshaller() );
         *
         */
        bool addMarshaller( RTT::Marshaller* headerM, RTT::Marshaller* bodyM);

        /**
         * Remove and delete all added Marshallers.
         */
        bool removeMarshallers();

        /**
         * @name Script Methods
         * @{
         */

        /**
         * Implementation of TaskCore::configureHook().
         * Calls load().
         */
        virtual bool configureHook();

        /**
         * Implementation of TaskCore::cleanupHook().
         * Calls store() and clears the reporting configuration.
         */
        virtual void cleanupHook();

        /**
         * Read the configuration file in order to decide
         * on what to report.
         */
        bool load();

        /**
         * Write the configuration file describing what is
         * being reported.
         */
        bool store();

        /**
         * Write state information of a component. This method must be
         * overridden by a subclass to be useful.
         */
        virtual bool screenComponent( const std::string& comp );

        /**
         * Report all the data ports of a component.
         */
        bool reportComponent( const std::string& component );

        /**
         * Unreport the data ports of a component.
         */
        bool unreportComponent( const std::string& component );

        /**
         * Report a specific data port of a component.
         */
        bool reportPort(const std::string& component, const std::string& port );

        /**
         * Unreport a specific data port of a component.
         */
        bool unreportPort(const std::string& component, const std::string& port );

        /**
         * Report a specific data source of a component.
         */
        bool reportData(const std::string& component,const std::string& dataname);

        /**
         * Unreport a specific data source of a component.
         */
        bool unreportData(const std::string& component,const std::string& datasource);

        /**
         * This real-time function makes copies of the data to be
         * reported.
         */
        void snapshot();

        void cleanReport();

        /** @} */

    protected:
        typedef boost::tuple<std::string,
                             RTT::DataSourceBase::shared_ptr,
                             boost::shared_ptr<RTT::CommandInterface>,
                             RTT::DataSourceBase::shared_ptr,
                             std::string> DTupple;
        /**
         * Stores the 'datasource' of all reported items as properties.
         */
        typedef std::vector<DTupple> Reports;
        Reports root;

        bool reportDataSource(std::string tag, std::string type, RTT::DataSourceBase::shared_ptr orig);

        bool unreportDataSource(std::string tag);

        virtual bool startHook();

        void makeReport();

        /**
         * This not real-time function processes the copied data.
         */
        virtual void updateHook();

        virtual void stopHook();

        typedef std::vector< std::pair<boost::shared_ptr<RTT::Marshaller>, boost::shared_ptr<RTT::Marshaller> > > Marshallers;
        Marshallers marshallers;
        RTT::PropertyBag report;

        RTT::Property<bool>          autotrigger;
        RTT::Property<std::string>   config;
        RTT::Property<bool>          writeHeader;
        RTT::Property<bool>          decompose;

        RTT::TimeService::ticks starttime;
        RTT::Property<RTT::TimeService::Seconds> timestamp;

    };

}

#endif
