/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:20 CET 2004  NiceHeaderMarshaller.hpp

                        NiceHeaderMarshaller.hpp -  description
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
 *   produce an execunice, this file does not by itself cause the         *
 *   resulting execunice to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the execunice file might be covered by the GNU General   *
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

#ifndef PI_PROPERTIES_NICEHEADER_SERIALIZER
#define PI_PROPERTIES_NICEHEADER_SERIALIZER

#include <rtt/Property.hpp>
#include <rtt/marsh/StreamProcessor.hpp>
#include <rtt/marsh/MarshallInterface.hpp>

namespace RTT
{
    /**
     * A marsh::MarshallInterface for generating headers usable for interpretation by
     * plot programs. A header looks like:
     * @verbatim
     * # Col_1_name Col_2_name Col_3_name ....
     * @endverbatim
     */
    template<typename o_stream>
    class NiceHeaderMarshaller
    : public marsh::MarshallInterface, public marsh::StreamProcessor<o_stream>
    {
        bool did_comment;
        int nameless_counter;
        std::string prefix;
        public:
            typedef o_stream output_stream;
            typedef o_stream OutputStream;

            NiceHeaderMarshaller(output_stream &os) :
                marsh::StreamProcessor<o_stream>(os),
                did_comment(false), nameless_counter(0)
            {
            }

            virtual ~NiceHeaderMarshaller() {}

			virtual void serialize(base::PropertyBase* v)
			{
                Property<PropertyBag>* bag = dynamic_cast< Property<PropertyBag>* >( v );
                if ( bag )
                    this->serialize( *bag );
                else
                    store( v->getName() );
			}


            virtual void serialize(const PropertyBag &v)
			{
                // start with a comment sign.
                if (did_comment == false )
                    *this->s << "";
                did_comment = true;

                for (
                    PropertyBag::const_iterator i = v.getProperties().begin();
                    i != v.getProperties().end();
                    i++ )
                {
                    this->serialize(*i);
                }
			}

            /**
             * @return the number of characters on this line.
             */
            void store(const std::string& name)
            {
                if ( name.empty() )
                    ++nameless_counter;
                else
                    nameless_counter = 0;
                if ( !prefix.empty() )
                    *this->s << ' ' << prefix << '.';
                else
                    *this->s << ' ';
                if ( nameless_counter )
                    *this->s << nameless_counter;
                else
                    *this->s << name;
            }

            virtual void serialize(const Property<PropertyBag> &v)
			{
                if ( v.rvalue().empty() )
                    store( v.getName() + "[0]" );
                else {
                    std::string oldpref = prefix;
                    if ( prefix.empty() )
                        prefix = v.getName();
                    else
                        prefix += '.' + v.getName();

                    serialize(v.rvalue());

                    prefix = oldpref;
                    nameless_counter = 0;
                }
            }

            virtual void flush()
            {
                did_comment = false;
                nameless_counter = 0;
                *this->s << std::endl;
                this->s->flush();

            }
	};
}
#endif
