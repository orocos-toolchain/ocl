#ifndef PI_PROPERTIES_NETCDFHEADER_SERIALIZER
#define PI_PROPERTIES_NETCDFHEADER_SERIALIZER

#include <rtt/Property.hpp>
#include <boost/lexical_cast.hpp>

#include <netcdf.h>

#define DIMENSION_VAR 1
#define DIMENSION_ARRAY 2

#include <iostream>
using namespace std;

namespace RTT
{
    /**
     * A marsh::MarshallInterface for generating variables in a netcdf dataset
     */
    class NetcdfHeaderMarshaller 
    : public marsh::MarshallInterface
    {
      int nameless_counter;
      std::string prefix;
      int ncid;
      int dimsid;
      int ncopen;

      public:

      NetcdfHeaderMarshaller(int ncid, int dimsid) : ncid( ncid ), dimsid(dimsid), ncopen(0) {}

      virtual ~NetcdfHeaderMarshaller() {}

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
          if (Ps)
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
        int retval;

        /**
         * Check if the netcdf file is already in define mode.
         * Increase counter every time serialize function is called and no errors occurs.
         */
        if ( ncopen ) {
          ncopen++;
        }
        else {
          retval = nc_redef(ncid);
           if ( retval )
             log(Error) << "Could not enter define mode in NetcdfHeaderMarshaller, error "<< retval <<endlog();
           else
             ncopen++;
        }
        
        for (
          PropertyBag::const_iterator i = v.getProperties().begin();
          i != v.getProperties().end();
          i++ )
            {                 
              this->serialize(*i);
            }

        /**
         * Decrease counter, if zero enter data mode else stay in define mode 
         */
        if (--ncopen)
          log(Info) << "Serializer still in progress" <<endlog();
        else {
          retval = nc_enddef(ncid);
           if (retval)
             log(Error) << "Could not leave define mode, error" << retval <<endlog();
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
       * Create a variable of data type char
       */
      void store(Property<char> *v)
      {
        int retval;
        int varid;
        std::string sname = composeName(v->getName());

        /**
         * Create a new variable with only one dimension i.e. the unlimited time dimension
         */ 
        retval = nc_def_var(ncid, sname.c_str(), NC_BYTE, DIMENSION_VAR,
                    &dimsid, &varid);
        if ( retval )
          log(Error) << "Could not create variable " << sname << ", error " << retval <<endlog();
        else
          log(Info) << "Variable "<< sname << " successfully created" <<endlog();
      }

      /**
       * Create a variable of data type short
       */
      void store(Property<short> *v)
      {
        int retval;
        int varid;
        std::string sname = composeName(v->getName());

        /**
         * Create a new variable with only one dimension i.e. the unlimited time dimension
         */ 
        retval = nc_def_var(ncid, sname.c_str(), NC_SHORT, DIMENSION_VAR,
                    &dimsid, &varid);
        if ( retval )
          log(Error) << "Could not create variable " << sname << ", error " << retval <<endlog();
        else
          log(Info) << "Variable "<< sname << " successfully created" <<endlog();
      }

      /**
       * Create a variable of data type int
       */
      void store(Property<int> *v)
      {
        int retval;
        int varid;
        std::string sname = composeName(v->getName());

        /**
         * Create a new variable with only one dimension i.e. the unlimited time dimension
         */ 
        retval = nc_def_var(ncid, sname.c_str(), NC_INT, DIMENSION_VAR,
                    &dimsid, &varid);
        if ( retval )
          log(Error) << "Could not create variable " << sname << ", error " << retval <<endlog();
        else
          log(Info) << "Variable "<< sname << " successfully created" <<endlog();
      }

      /**
       * Create a variable of data type float
       */
      void store(Property<float> *v)
      {
        int retval;
        int varid;
        std::string sname = composeName(v->getName());

        /**
         * Create a new variable with only one dimension i.e. the unlimited time dimension
         */ 
        retval = nc_def_var(ncid, sname.c_str(), NC_FLOAT, DIMENSION_VAR,
                    &dimsid, &varid);
        if ( retval )
          log(Error) << "Could not create variable " << sname << ", error " << retval <<endlog();
        else
          log(Info) << "Variable "<< sname << " successfully created" <<endlog();
      }

      /**
       * Create a variable of data type double
       */
      void store(Property<double> *v)
      {
        int retval;
        int varid;
        std::string sname = composeName(v->getName());

        /**
         * Create a new variable with only one dimension i.e. the unlimited time dimension
         */ 
        retval = nc_def_var(ncid, sname.c_str(), NC_DOUBLE, DIMENSION_VAR,
                    &dimsid, &varid);

        if ( retval )
          log(Error) << "Could not create variable " << sname << ", error " << retval <<endlog();
        else
          log(Info) << "Variable "<< sname << " successfully created" <<endlog();
      }

      /**
       * Create a variable with two dimensions of data type double 
       */
      void store(Property<std::vector<double> > *v)
      {
        int retval;
        int varid;

        std::string dim_name = v->getName().c_str();
        dim_name += "_dim";
        const char *dimname = dim_name.c_str();

        const char *name = v->getName().c_str();

        int dims[ DIMENSION_ARRAY ];
        int var_dim;

        // create new dimension
        retval = nc_def_dim(ncid, dimname, v->rvalue().size(), &var_dim);
        if ( retval )
          log(Error) << "Could not create new dimension for "<< dimname <<", error "<< retval <<endlog();

        // fill in dims
        dims[0] = dimsid;
        dims[1] = var_dim;

        retval = nc_def_var(ncid, name, NC_DOUBLE, DIMENSION_ARRAY,
                    dims, &varid);
        if ( retval )
          log(Error) << "Could not create " << name << ", error " << retval <<endlog();
        else
          log(Info) << "Variable "<< name << " successfully created" <<endlog();
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

      virtual void flush() {}
    };
}
#endif
