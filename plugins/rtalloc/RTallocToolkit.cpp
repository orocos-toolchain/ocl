/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include "RTallocToolkit.hpp"
#include <rtt/Types.hpp>
#include <rtt/TypeStream-io.hpp>
#include <rtt/TemplateTypeInfo.hpp>

namespace RTT
{
    using namespace RTT;
    using namespace RTT::detail;
    using namespace std;

    std::ostream& operator<<(std::ostream& os, const OCL::String& t)
	{
		os << t.c_str();
		return os;
	}

    std::istream& operator>>(std::istream& is, OCL::String& t)
	{
        // \todo
        is >> t;
		return is;
	}


    RTallocPlugin RTallocToolkit;

    std::string RTallocPlugin::getName()
    {
        return "RTalloc";
    }

    bool RTallocPlugin::loadTypes()
    {
        TypeInfoRepository::shared_ptr ti = TypeInfoRepository::Instance();

        ti->addType( new RTallocPtimeTypeInfo("rtstring") );

        return true;
    }

    bool RTallocPlugin::loadConstructors()
    {
		// no constructors

        // \todo constructors

        return true;
    }

    bool RTallocPlugin::loadOperators()
    {
		// no operators

        // \todo operators

        return true;
    }


    bool RTallocPtimeTypeInfo::decomposeTypeImpl(const OCL::String& source, PropertyBag& targetbag)
    {
        targetbag.setType("rtstring");
        // \todo decompose
		assert(0);
        return true;
    }

    bool RTallocPtimeTypeInfo::composeTypeImpl(const PropertyBag& bag, OCL::String& result)
    {
        if ( "rtstring" == bag.getType() ) // ensure is correct type
        {
            // \todo compose
			assert(0);
        }
        return false;
    }

}

ORO_TOOLKIT_PLUGIN(RTT::RTallocToolkit)
