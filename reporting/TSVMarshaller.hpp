/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:20 CET 2004  TSVMarshaller.hpp

                        TSVMarshaller.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef PI_PROPERTIES_TABLESERIALIZER
#define PI_PROPERTIES_TABLESERIALIZER

#include <fstream>
#include <map>
#include <rtt/Property.hpp>
#include <rtt/base/PropertyIntrospection.hpp>
#include <rtt/marsh/MarshallInterface.hpp>

namespace RTT {

/**
 * A marsh::MarshallInterface for generating a stream of numbers, ordered in
 * columns. A new row is created on each flush() command. The
 * TableHeaderMarshaller can create the appropriate heading for
 * the columns.
 */
class TSVMarshaller : public marsh::MarshallInterface {
  std::string msep;
  std::map<std::string, std::ofstream*> streams;
  typedef std::map<std::string, std::ofstream*>::iterator stream_it;
  double current_reporting_timestamp;

  std::ofstream* getStream(const std::string& key, base::PropertyBase* v) {
    if (streams.count(key) == 0) {
      streams[key] =
          new std::ofstream((key + ".txt").c_str(), std::ofstream::out);
      (*streams[key]) << "ReportingTimestamp";
      serialize(v, *streams[key], "", true);
      (*streams[key]) << std::endl;
    }
    return streams[key];
  }

 public:
  /**
   * Create a new marshaller, streaming the data to a stream.
   * @param os The stream to write the data to (i.e. cerr)
   * @param sep The separater to place between each column and at
   * the end of the line.
   */
  TSVMarshaller(std::string sep = " ") : msep(sep) {}

  virtual ~TSVMarshaller() {
    for (stream_it it = streams.begin(); it != streams.end(); ++it) {
      it->second->close();
      delete it->second;
    }
  }

  virtual void serialize(base::PropertyBase* v) {
    if (v->getName() == "TimeStamp") {
        Property<double>* timestamp = dynamic_cast<Property<double>* >(v);
        current_reporting_timestamp = timestamp->rvalue();
    } else {
      std::ofstream& stream = *getStream(v->getName(), v);
      stream << current_reporting_timestamp;
      serialize(v, stream);
      stream << std::endl;
    }
  }

  void serialize(base::PropertyBase* v, std::ofstream& stream,
                 const std::string& prefix = "", bool header = false) {
    Property<PropertyBag>* bag = dynamic_cast<Property<PropertyBag>*>(v);
    if (bag) {
      serialize(bag->rvalue(), stream, bag->getName(), header);
    } else {
      if (header) {
        stream << msep << v->getName();
      } else {
        stream << msep << v->getDataSource();
      }
    }
  }

  virtual void serialize(const PropertyBag& v) {
    for (PropertyBag::const_iterator i = v.getProperties().begin();
         i != v.getProperties().end(); i++) {
      serialize(*i);
    }
  }

  virtual void serialize(const PropertyBag& v, std::ofstream& stream,
                         const std::string& prefix = "", bool header = false) {
    for (PropertyBag::const_iterator i = v.getProperties().begin();
         i != v.getProperties().end(); i++) {
      this->serialize(*i, stream, prefix, header);
    }
  }

  virtual void flush() {
    for (stream_it it = streams.begin(); it != streams.end(); ++it) {
      it->second->flush();
    }
  }
};
}
#endif
