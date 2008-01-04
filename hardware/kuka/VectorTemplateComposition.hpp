/***************************************************************************
  tag: Tinne De Laet Soetens  2007  VectorTemplateComposition.hpp 
       2007 Ruben Smits 
                        VectorTemplateComposition.hpp -  description
 
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
 
#ifndef VECTOR_TEMPLATE_COMPOSITION_HPP
#define VECTOR_TEMPLATE_COMPOSITION_HPP

#include <rtt/Property.hpp>
#include <rtt/PropertyBag.hpp>
#include <rtt/TemplateTypeInfo.hpp>
#include <rtt/Types.hpp>
#include <rtt/Logger.hpp>
#include <ostream>
#include <sstream>
#include <vector>

namespace RTT
{
    class PropertyIntrospection;

    /**
     * A decomposeProperty method for decomposing a vector<T>
     * into a PropertyBag with Property<T>'s.
     * The dimension of the vector must be less than 100 if you want the
     * Property<T>'s to have a different name.
     */
    template<class T> 
    void decomposeProperty(const std::vector<T>& vec, PropertyBag& targetbag)
    {
        targetbag.setType("vector");
        int dimension = vec.size();
        std::string str;
        
        assert( targetbag.empty() );

        for ( int i=0; i < dimension ; i++){
            std::stringstream out;
            out << i+1;
            str = out.str();
            targetbag.add( new Property<T>("Element " + str, str +"th element of vector",vec[i]) ); // Put variables in the bag
        }
    };
    /**
     * A composeProperty method for composing a property of a vector<T>
     * The dimension of the vector must be less than 100.
     */
    template<class T> 
    bool composeProperty(const PropertyBag& bag, std::vector<T>& result)
    {
        if ( bag.getType() == "vector" ) {
            int dimension = bag.size();
            Logger::log() << Logger::Info << "bag size " << dimension <<Logger::endl;
            result.resize( dimension );
            
            // Get values
            for (int i = 0; i < dimension ; i++) {
                PropertyBase* element = bag.getItem( i );
                //Property<PropertyBag>* t_bag= dynamic_cast< Property<PropertyBag>* >( element ); 
                log(Debug)<<element->getName()<<", "<< element->getDescription()<<endlog();
                //Property<T> my_property_t (t_abag->getName(),t_bag->getDescription());
                Property<T> my_property_t (element->getName(),element->getDescription());
                if(my_property_t.getType()!=element->getType())
                    log(Error)<< "Type of "<< element->getName() << " does not match type of vector"<<endlog();
                else{
                    my_property_t.getTypeInfo()->composeType(element->getDataSource(),my_property_t.getDataSource()); 
                    result[ i ] = my_property_t.get();
                }
            }
        }
        else {
            Logger::log() << Logger::Error << "Composing Property< std::vector<T> > :"
                          << " type mismatch, got type '"<< bag.getType()
                          << "', expected type 'vector'."<<Logger::endl;
            return false;
        }
        return true;
    };
  template <typename T>
    struct StdVectorTemplateTypeInfo
        : public TemplateContainerTypeInfo<std::vector<T>, int, T, ArrayIndexChecker<std::vector<T> >, SizeAssignChecker<std::vector<T> >, false >
    {
        StdVectorTemplateTypeInfo<T>( std::string name )
            : TemplateContainerTypeInfo<std::vector<T>, int, T, ArrayIndexChecker<std::vector<T> >, SizeAssignChecker<std::vector<T> >, false >(name)
        {
        };
        
        bool decomposeTypeImpl(const std::vector<T>& vec, PropertyBag& targetbag) const
        {
            decomposeProperty<T>( vec, targetbag );
            return true;
        };
        
        bool composeTypeImpl(const PropertyBag& bag, std::vector<T>& result) const
        {
            return composeProperty<T>( bag, result );
        }
        
    };
      
};
#endif

