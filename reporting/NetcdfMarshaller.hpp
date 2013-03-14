#ifndef PI_PROPERTIES_NETCDFTABLESERIALIZER
#define PI_PROPERTIES_NETCDFTABLESERIALIZER

#include <rtt/Property.hpp>
#include <rtt/base/PropertyIntrospection.hpp>
#include <boost/lexical_cast.hpp>

#include <netcdf.h>
#include <iostream>
using namespace std;

namespace RTT
{

    /**
     * A marsh::MarshallInterface for writing data logs into the variables of a netcdf file. 
     * The dimension of the time is increased on each flush() command. 
     * The NetcdfHeaderMarshaller creates the appropriate variables in a netcdf file.
     */
    class NetcdfMarshaller 
        : public marsh::MarshallInterface
    {
      int ncid;
      size_t index;
      int nameless_counter;
      std::string prefix;
      
      public:
        /**
         * Create a new NetcdfMarshaller
         * @param ncid The ID number of the netcdf file
         */
        NetcdfMarshaller(int ncid) :
          ncid ( ncid ) {index=0;}

        virtual ~NetcdfMarshaller() {}

        virtual void serialize(base::PropertyBase* v) 
        { 
         Property<PropertyBag>* bag = dynamic_cast< Property<PropertyBag>* >( v );
         if ( bag )
           this->serialize( *bag );
         else {
           Property<char>* Pc = dynamic_cast< Property<char>* >( v );
           if ( Pc )
             {
               store(Pc);
               return;
             }
           Property<short>* Ps = dynamic_cast< Property<short>* >( v );
           if ( Ps )
             {
               store(Ps);
               return;
             }
           Property<int>* Pi = dynamic_cast< Property<int>* >( v );
           if (Pi)
             {
               store(Pi);
               return;
             }
           Property<float>* Pf = dynamic_cast< Property<float>* >( v );
           if (Pf)
             {
               store(Pf);
               return;
             }
           Property<double>* Pd = dynamic_cast< Property<double>* >( v );
           if (Pd)
             {
               store(Pd);
               return;
             }
           Property<std::vector<double> >* Pv = dynamic_cast< Property<std::vector<double> >* >( v );
           if (Pv)
             {
               store(Pv);
               return;
             }
         }
        }
                        
        virtual void serialize(const PropertyBag &v) 
        {
          for (
            PropertyBag::const_iterator i = v.getProperties().begin();
            i != v.getProperties().end();
            i++ )
            {
              this->serialize( *i );
            }
        }

        virtual void serialize(const Property<PropertyBag> &v) 
        {
          std::string oldpref = prefix;

          // Avoid a "." in the beginning of a variable name
          if(prefix.empty())
            prefix = v.getName();
          else
            prefix += "." + v.getName();

          serialize(v.rvalue());

          prefix = oldpref;
          nameless_counter = 0;
        }

        /**
         * Write char data to corresponding variable name
         */
        void store(Property<char> *v)
        {
          int retval;
          int varid;
          signed char value = v->rvalue();
          std::string sname = composeName(v->getName());

          /**
           * Get netcdf variable ID from name
           */
          retval = nc_inq_varid(ncid, sname.c_str(), &varid);
          if (retval)
            log(Error) << "Could not get variable id of " << sname << ", error " << retval <<endlog();

          /**
           * Write a single data value
           */
          retval = nc_put_var1_schar(ncid, varid, &index, &value);
          if(retval)
            log(Error) << "Could not write variable " << sname << ", error " << retval <<endlog();
        }

        /**
         * Write short data to corresponding variable name
         */
        void store(Property<short> *v)
        {

          int retval;
          int varid;
          short value = v->rvalue();
          std::string sname = composeName(v->getName());

          /**
           * Get netcdf variable ID from name
           */
          retval = nc_inq_varid(ncid, sname.c_str(), &varid);
          if (retval)
            log(Error) << "Could not get variable id of " << sname << ", error " << retval <<endlog();

          /**
           * Write a single data value
           */
          retval = nc_put_var1_short(ncid, varid, &index, &value);
          if(retval)
            log(Error) << "Could not write variable " << sname << ", error " << retval <<endlog();
        }

        /**
         * Write int data to corresponding variable name
         */
        void store(Property<int> *v)
        {
          int retval;
          int varid;
          int value = v->rvalue();
          std::string sname = composeName(v->getName());

          /**
           * Get netcdf variable ID from name
           */
          retval = nc_inq_varid(ncid, sname.c_str(), &varid);
          if (retval)
            log(Error) << "Could not get variable id of " << sname << ", error " << retval <<endlog();

          /**
           * Write a single data value
           */
          retval = nc_put_var1_int(ncid, varid, &index, &value);
          if(retval)
            log(Error) << "Could not write variable " << sname << ", error " << retval <<endlog();
        }

        /**
         * Write float data to corresponding variable name
         */
        void store(Property<float> *v)
        {
          int retval;
          int varid;
          float value = v->rvalue();
          std::string sname = composeName(v->getName());

          /**
           * Get netcdf variable ID from name
           */
          retval = nc_inq_varid(ncid, sname.c_str(), &varid);
          if (retval)
            log(Error) << "Could not get variable id of " << sname << ", error " << retval <<endlog();

          /**
           * Write a single data value
           */
          retval = nc_put_var1_float(ncid, varid, &index, &value);
          if(retval)
            log(Error) << "Could not write variable " << sname << ", error " << retval <<endlog();

        }

        /**
         * Write double data to corresponding variable name
         */
        void store(Property<double> *v)
        {
          int retval;
          int varid;
          double value = v->rvalue();
          std::string sname = composeName(v->getName());

          /**
           * Get netcdf variable ID from name
           */
          retval = nc_inq_varid(ncid, sname.c_str(), &varid);
          if (retval)
            log(Error) << "Could not get variable id of " << sname << ", error " << retval <<endlog();

          /**
           * Write a single data value
           */
          retval = nc_put_var1_double(ncid, varid, &index, &value);
          if(retval)
            log(Error) << "Could not write variable " << sname << ", error " << retval <<endlog();
        }    

        /**
         * Write double array data into corresponding variable name
         */
        void store(Property<std::vector<double> > *v)
        {
          int retval;
          int varid;
          const char *name = v->getName().c_str();
          size_t start[2], count[2];

          /**
           * Specify index where the first data will be written
           */
          start[0] = index; start[1] = 0;
          /**
           * Specify the number of values that will be written in each dimension
           */
          count[0] = 1; count[1] = v->rvalue().size();

          retval = nc_inq_varid(ncid, name, &varid);
          if (retval)
            log(Error) << "Could not get variable id of " << name << ", error " << retval <<endlog();

          retval = nc_put_vara_double(ncid, varid, start, count, &(v->rvalue().front()));
          if(retval)
            log(Error) << "Could not write variable " << name << ", error " << retval <<endlog();

        }

        std::string composeName(std::string propertyName)
        {
          std::string last_name;

          if( propertyName.empty() ) {
            nameless_counter++;
            last_name = boost::lexical_cast<std::string>( nameless_counter );
          }
          else {
            nameless_counter = 0;
            last_name = propertyName;
          }
          if ( prefix.empty() )
          	return last_name;
          else
          	return prefix + "." + last_name;
        }

        /**
         * Increase unlimited time dimension
         */
        virtual void flush() 
        {
          index++;
        }

     };
}
#endif
