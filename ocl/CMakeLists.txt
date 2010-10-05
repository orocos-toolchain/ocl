
# This gathers all the .cpp files into the variable 'SRCS'
FILE( GLOB HPPS [^.]*.hpp )
FILE( GLOB SRCS [^.]*.cpp )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

GLOBAL_ADD_COMPONENT( orocos-ocl-common ${SRCS} )
GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
