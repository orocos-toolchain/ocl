
# This gathers all the .cpp files into the variable 'SRCS'
FILE( GLOB HPPS [^.]*.hpp )
FILE( GLOB SRCS [^.]*.cpp )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

orocos_component( orocos-ocl-common HMIConsoleOutput.cpp EmptyTaskContext.cpp INSTALL lib/orocos)
orocos_typekit( ocl ocltoolkit.cpp )
orocos_service( print PrintService.cpp )

GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
