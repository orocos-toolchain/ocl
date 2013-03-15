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
#include <rtt/marsh/MarshallInterface.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/TaskContext.hpp>

#include <rtt/RTT.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @brief A Component for periodically reporting Component
     * Port contents to a human readable text format. The
     * default format is a table with a header.
     *
     * It can report to any data format, using the 'addMarshaller'
     * function, which is typically done in sub classes of this
     * component.
     *
     * @par Configuration
     * The ReportingComponent is configured using
     * its properties. For example, to enable writing a header or
     * not. The ReportData struct describes which ports and peer components need
     * to be monitored. You need to load properties into this struct
     * (see RTT::Marshalling service).
     * This struct can be filled in as such:
     *
     * @code
     <?xml version="1.0" encoding="UTF-8"?>
     <!DOCTYPE properties SYSTEM "cpf.dtd">
     <properties>
        <!-- ... other component properties .. -->

        <struct name="ReportData" type="PropertyBag">
           <!-- Monitor all ports of a Component : -->
           <simple name="Component" type="string"><description></description><value>ComponentX</value></simple>

           <!-- Monitor a single Data or base::Buffer-Port of another Component : -->
           <simple name="Port" type="string"><description></description><value>ComponentY.PortZ</value></simple>
           <!-- add as many lines as desired... -->
        </struct>
     </properties>
     @endcode
     *
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
        bool addMarshaller( RTT::marsh::MarshallInterface* headerM, RTT::marsh::MarshallInterface* bodyM);

        /**
         * Remove and delete all added Marshallers.
         */
        bool removeMarshallers();

        /**
         * @name Script Methods
         * @{
         */

        /**
         * Implementation of base::TaskCore::configureHook().
         * Calls load().
         */
        virtual bool configureHook();

        /**
         * Implementation of base::TaskCore::cleanupHook().
         * Calls store() and clears the reporting configuration.
         */
        virtual void cleanupHook();

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
         * @return true if new data is available.
         */
        bool copydata();

        /**
         * Copy the reported data and trigger the generation of a sampling line.
         */
        void snapshot();

        void cleanReport();

        /** @} */

    protected:
        /**
         * tuple that describes each sample. Uses get<N>() to read it:
         * @0 The qualified name of the data (componentname.portname)
         * @1 The data source of the data. Always returns the last value.
         * @2 A copy command to copy from this data source to another one
         * @3 The target data source for the copy operation
         * @4 The type of the data, "Data" (props and attrs) or "Port".
         * @5 'newdata': The new data flag. Flags if the DataSource contains new data.
         * @6 'tracked': True if this source may lead to additional data. If false, the source
         * in itself will not cause to a re-scan. Used in copydata() to allow ports
         * to be rescanned, while props and attrs never cause this (they always have newdata,
         * but this is ignored).
         */
        typedef boost::tuple<std::string,
                             RTT::base::DataSourceBase::shared_ptr,
                             std::string,RTT::base::PropertyBase*,RTT::base::InputPortInterface*,bool,bool> DTupple;

        //! Use these to index DTupple objects.
        typedef enum { T_QualName = 0, T_PortDS, T_DataType, T_Property, T_Port, T_NewData, T_Tracked } T_Types;
        /**
         * Stores the 'datasource' of all reported items as properties.
         */
        typedef std::vector<DTupple> Reports;
        Reports root;

        bool reportDataSource(std::string tag, std::string type, RTT::base::DataSourceBase::shared_ptr origm, RTT::base::InputPortInterface* ipi, bool);

        bool unreportDataSource(std::string tag);

        virtual bool startHook();

        void makeReport2();

        /**
         * This not real-time function processes the copied data.
         */
        virtual void updateHook();

        virtual void stopHook();

        typedef std::vector< std::pair<boost::shared_ptr<RTT::marsh::MarshallInterface>, boost::shared_ptr<RTT::marsh::MarshallInterface> > > Marshallers;
        Marshallers marshallers;
        RTT::PropertyBag report;

        /**
         * Used to communicate between snapshot() and updateHook()
         * if updateHook needs to make a copy.
         */
        bool snapshotted;
        RTT::Property<std::string>   config;
        RTT::Property<bool>          writeHeader;
        RTT::Property<bool>          decompose;
        RTT::Property<bool>          insnapshot;
        RTT::Property<bool>          synchronize_with_logging;
        RTT::Property<PropertyBag>   report_data;
        RTT::ConnPolicy              report_policy;
        bool                         onlyNewData;

        RTT::os::TimeService::ticks starttime;
        RTT::Property<RTT::os::TimeService::Seconds> timestamp;
        //! If false, a sequence size has changed.
        RTT::internal::DataSource<bool>::shared_ptr mchecker;

    };

}

#endif
