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
#include <rtt/types/TemplateTypeInfo.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/Logger.hpp>
#include <rtt/internal/DataSources.hpp>
#include <ostream>
#include <sstream>
#include <vector>

namespace RTT
{

    /**
     * A decomposeProperty method for decomposing a vector<T>
     * into a RTT::PropertyBag with RTT::Property<T>'s.
     * The dimension of the vector must be less than 100 if you want the
     * RTT::Property<T>'s to have a different name.
     */
    template<class T>
    void decomposeProperty(const std::vector<T>& vec, RTT::PropertyBag& targetbag)
    {
        std::string tname = internal::DataSourceTypeInfo<T>::getType();
        targetbag.setType(tname+"s");
        int dimension = vec.size();
        std::string str;

        assert( targetbag.empty() );

        for ( int i=0; i < dimension ; i++){
            std::stringstream out;
            out << i+1;
            str = out.str();
            targetbag.add( new RTT::Property<T>("Element " + str, str +"th element of list",vec[i]) ); // Put variables in the bag
        }
    };
    /**
     * A composeProperty method for composing a property of a vector<T>
     * The dimension of the vector must be less than 100.
     */
    template<class T>
    bool composeProperty(const RTT::PropertyBag& bag, std::vector<T>& result)
    {
        std::string tname = internal::DataSourceTypeInfo<T>::getType();
        
        if ( bag.getType() == tname+"s" ) {
            int dimension = bag.size();
            RTT::Logger::log() << RTT::Logger::Info << "bag size " << dimension <<RTT::Logger::endl;
            result.resize( dimension );

            // Get values
            for (int i = 0; i < dimension ; i++) {
                base::PropertyBase* element = bag.getItem( i );
                //RTT::Property<RTT::PropertyBag>* t_bag= dynamic_cast< RTT::Property<RTT::PropertyBag>* >( element );
                log(Debug)<<element->getName()<<", "<< element->getDescription()<<endlog();
                //RTT::Property<T> my_property_t (t_abag->getName(),t_bag->getDescription());
                RTT::Property<T> my_property_t (element->getName(),element->getDescription());
                if(my_property_t.getType()!=element->getType())
                    log(Error)<< "Type of "<< element->getName() << " does not match type of "<<tname+"s"<<endlog();
                else{
                    my_property_t.getTypeInfo()->composeType(element->getDataSource(),my_property_t.getDataSource());
                    result[ i ] = my_property_t.get();
                }
            }
        }
        else {
            RTT::Logger::log() << RTT::Logger::Error << "Composing RTT::Property< std::vector<T> > :"
                          << " type mismatch, got type '"<< bag.getType()
                          << "', expected type "<<tname<<"s."<<RTT::Logger::endl;
            return false;
        }
        return true;
    };

    template <typename T, bool has_ostream>
    struct types::StdVectorTemplateTypeInfo
        : public types::TemplateContainerTypeInfo<std::vector<T>, int, T, types::ArrayIndexChecker<std::vector<T> >, types::SizeAssignChecker<std::vector<T> >, has_ostream >
    {
        types::StdVectorTemplateTypeInfo<T,has_ostream>( std::string name )
            : types::TemplateContainerTypeInfo<std::vector<T>, int, T, types::ArrayIndexChecker<std::vector<T> >, types::SizeAssignChecker<std::vector<T> >, has_ostream >(name)
        {
        };

        bool decomposeTypeImpl(const std::vector<T>& vec, RTT::PropertyBag& targetbag) const
        {
            decomposeProperty<T>( vec, targetbag );
            return true;
        };

        bool composeTypeImpl(const RTT::PropertyBag& bag, std::vector<T>& result) const
        {
            return composeProperty<T>( bag, result );
        }

    };
    
    template<typename T>
    std::ostream& operator << (std::ostream& os, const std::vector<T>& vec)
    {
        os<<'[';
        for(unsigned int i=0;i<vec.size();i++){
            if(i>0)
                os<<',';
            os<<vec[i]<<' ';
        }
        
        return os << ']';
    };
    
    template<typename T>
    std::istream& operator >> (std::istream& is,std::vector<T>& vec)
    {
        return is;
    };

    template<typename T>
    struct types::stdvector_ctor
        : public std::unary_function<int, const std::vector<T>&>
    {
        typedef const std::vector<T>& (Signature)( int );
        mutable boost::shared_ptr< std::vector<T> > ptr;
        types::stdvector_ctor()
            : ptr( new std::vector<T>() ) {}
        const std::vector<T>& operator()( int size ) const
        {
            ptr->resize( size );
            return *(ptr);
        }
    };
    
    /**
     * See internal::NArityDataSource which requires a function object like
     * this one.
     */
    template<typename T>
    struct types::stdvector_varargs_ctor
    {
        typedef const std::vector<T>& result_type;
        typedef T argument_type;
        result_type operator()( const std::vector<T>& args ) const
        {
            return args;
        }
    };
    
    /**
     * Constructs an array with \a n elements, which are given upon
     * construction time.
     */
     template<typename T>
     struct types::StdVectorBuilder
         : public types::TypeBuilder
     {
         virtual base::DataSourceBase::shared_ptr build(const std::vector<base::DataSourceBase::shared_ptr>& args) const {
             if (args.size() == 0 )
                 return base::DataSourceBase::shared_ptr();
             typename internal::NArityDataSource<types::stdvector_varargs_ctor<T> >::shared_ptr vds = new internal::NArityDataSource<types::stdvector_varargs_ctor<T> >();
             for(unsigned int i=0; i != args.size(); ++i) {
                 typename internal::DataSource<T>::shared_ptr dsd = internal::AdaptDataSource<T>()( args[i] );
                 if (dsd)
                     vds->add( dsd );
                 else
                     return base::DataSourceBase::shared_ptr();
             }
             return vds;
         }
     };

    template<typename T>
    struct types::stdvector_ctor2
        : public std::binary_function<int, T, const std::vector<T>&>
    {
        typedef const std::vector<T>& (Signature)( int, T );
        mutable boost::shared_ptr< std::vector<T> > ptr;
        types::stdvector_ctor2()
            : ptr( new std::vector<T>() ) {}
        const std::vector<T>& operator()( int size, T value ) const
        {
            ptr->resize( size );
            ptr->assign( size, value );
            return *(ptr);
        }
    };

    template<typename T>
    struct types::stdvector_index
        : public std::binary_function<const std::vector<T>&, int, T>
    {
        T operator()(const std::vector<T>& v, int index) const
        {
            if ( index >= (int)(v.size()) || index < 0)
                return T();
            return v[index];
        }
    };    

    template<class T>
    struct types::get_size
        : public std::unary_function<T, int>
    {
        int operator()(T cont ) const
        {
            return cont.size();
        }
    };

};
#endif

