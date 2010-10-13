#include "@pkgname@-component.hpp"
#include <ocl/Component.hpp>

/*
 * Using this macro, only one component may live
 * in one library. If you have put your component
 * in a namespace, don't forget to add it here too:
 */
ORO_CREATE_COMPONENT(@Pkgname@)
