/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include <rtt/corba/CorbaConversion.hpp>
#include "rtalloc/String.hpp"

// must be in RTT namespace to match some rtt/corba code
namespace RTT
{


template<>
struct AnyConversion< OCL::String >
{
    typedef const char*     CorbaType;
    typedef OCL::String     StdType;

    static CorbaType toAny(const StdType& orig) {
        return orig.c_str();
    }

    static StdType get(const CorbaType orig) {
        return StdType(orig);
    }

    static bool update(const CORBA::Any& any, StdType& ret) {
        CorbaType orig;
        if ( any >>= orig ) 
        {
            ret = orig;
            return true;
        }
        return false;
    }

    static CORBA::Any_ptr createAny( const StdType& t ) {
        CORBA::Any_ptr ret = new CORBA::Any();
        *ret <<= toAny( t );
        return ret;
    }
};


// namespace
};


