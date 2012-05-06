#ifndef OROCOS_@PKGNAME@_COMPONENT_HPP
#define OROCOS_@PKGNAME@_COMPONENT_HPP

#include <rtt/RTT.hpp>
#include <iostream>

class @Pkgname@
    : public RTT::TaskContext
{
 public:
    @Pkgname@(std::string const& name)
        : TaskContext(name)
    {
        std::cout << "@Pkgname@ constructed !" <<std::endl;
    }

    bool configureHook() {
        std::cout << "@Pkgname@ configured !" <<std::endl;
        return true;
    }

    bool startHook() {
        std::cout << "@Pkgname@ started !" <<std::endl;
        return true;
    }

    void updateHook() {
        std::cout << "@Pkgname@ executes updateHook !" <<std::endl;
    }

    void stopHook() {
        std::cout << "@Pkgname@ executes stopping !" <<std::endl;
    }

    void cleanupHook() {
        std::cout << "@Pkgname@ cleaning up !" <<std::endl;
    }
};

#endif
