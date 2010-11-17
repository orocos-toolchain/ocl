
# This gathers all the .cpp files into the variable 'SRCS'
FILE( GLOB HPPS [^.]*.hpp )
FILE( GLOB SRCS [^.]*.cpp )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

orocos_component( orocos-ocl-common ${SRCS} INSTALL lib/orocos)
GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
