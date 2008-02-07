#include <rtt/os/StartStopManager.hpp>
#include <rtt/TemplateTypeInfo.hpp>
#include "VectorTemplateComposition.hpp"

namespace OCL
{
    using namespace RTT;
    using namespace RTT::detail;
    
    int loadOCL()
    {
        RTT::TypeInfoRepository::Instance()->addType( new StdVectorTemplateTypeInfo<std::string>("stringList") );
        return true;
    }
    
    OS::InitFunction OCLLoader(&loadOCL);
}


