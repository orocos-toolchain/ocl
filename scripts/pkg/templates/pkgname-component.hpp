#ifndef OROCOS_@PKGNAME@_COMPONENT_HPP
#define OROCOS_@PKGNAME@_COMPONENT_HPP

#include <rtt/RTT.hpp>

class @Pkgname@ : public RTT::TaskContext{
  public:
    @Pkgname@(std::string const& name);
    bool configureHook();
    bool startHook();
    void updateHook();
    void stopHook();
    void cleanupHook();
};
#endif
