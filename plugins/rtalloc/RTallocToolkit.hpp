/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/

#ifndef __RTALLOC_TOOLKIT_HPP
#define __RTALLOC_TOOLKIT_HPP 1

#include <rtt/types/TypekitRepository.hpp>
#include <rtt/PropertyBag.hpp>
#include <rtt/types/TemplateTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>
#include <iostream>

#include "rtalloc/String.hpp"

namespace RTT
{
   
// these have to be in RTT namespace to work with the types::TypekitRepository

/// put the time onto the stream
std::ostream& operator<<(std::ostream& os, const OCL::String& s);
/// get a time from the stream
std::istream& operator>>(std::istream& is, OCL::String& s);


/**
 * This interface defines the types that we can pass around within Orocos.
 */
class RTallocPlugin : public RTT::types::TypekitPlugin
{
public:
    virtual std::string getName();

    virtual bool loadTypes();
    virtual bool loadConstructors();
    virtual bool loadOperators();
};

/// Provide OCL::String type to RTT type system
struct RTallocPtimeTypeInfo : 
        public RTT::types::TemplateTypeInfo<OCL::String,true> 
{
    RTallocPtimeTypeInfo(std::string name) :
            RTT::types::TemplateTypeInfo<OCL::String,true>(name)
    {};
    bool decomposeTypeImpl(const OCL::String& img, RTT::PropertyBag& targetbag);
    bool composeTypeImpl(const RTT::PropertyBag& bag, OCL::String& img);
};

// namespace
}

#endif
