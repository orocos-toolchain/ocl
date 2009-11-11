/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxstephen AT theptrgroupxxx DOT comxxx>
                               (remove the x's above)
 ***************************************************************************/
#include <ocl/ComponentLoader.hpp>
#include "send.hpp"
#include "recv.hpp"

// separate from implementation, so can directly include the .cpp files
// into an executable as well as include _all_ components into a 
// shared library.
ORO_CREATE_COMPONENT_TYPE();
ORO_LIST_COMPONENT_TYPE(Send);
ORO_LIST_COMPONENT_TYPE(Recv);
