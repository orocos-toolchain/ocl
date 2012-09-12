#include "@pkgname@-component.hpp"
#include <rtt/Component.hpp>
#include <iostream>

@Pkgname@::@Pkgname@(std::string const& name) : TaskContext(name){
  std::cout << "@Pkgname@ constructed !" <<std::endl;
}

bool @Pkgname@::configureHook(){
  std::cout << "@Pkgname@ configured !" <<std::endl;
  return true;
}

bool @Pkgname@::startHook(){
  std::cout << "@Pkgname@ started !" <<std::endl;
  return true;
}

void @Pkgname@::updateHook(){
  std::cout << "@Pkgname@ executes updateHook !" <<std::endl;
}

void @Pkgname@::stopHook() {
  std::cout << "@Pkgname@ executes stopping !" <<std::endl;
}

void @Pkgname@::cleanupHook() {
  std::cout << "@Pkgname@ cleaning up !" <<std::endl;
}

/*
 * Using this macro, only one component may live
 * in one library *and* you may *not* link this library
 * with another component library. Use
 * ORO_CREATE_COMPONENT_TYPE()
 * ORO_LIST_COMPONENT_TYPE(@Pkgname@)
 * In case you want to link with another library that
 * already contains components.
 *
 * If you have put your component class
 * in a namespace, don't forget to add it here too:
 */
ORO_CREATE_COMPONENT(@Pkgname@)
