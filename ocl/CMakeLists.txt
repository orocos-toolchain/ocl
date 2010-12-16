
# This gathers all the .cpp files into the variable 'SRCS'
FILE( GLOB HPPS [^.]*.hpp )
FILE( GLOB SRCS [^.]*.cpp )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

orocos_component( orocos-ocl-common HMIConsoleOutput.cpp EmptyTaskContext.cpp INSTALL lib/orocos)
orocos_typekit( ocl ocltoolkit.cpp INSTALL lib/orocos${OROCOS_SUFFIX}/types )
orocos_service( print PrintService.cpp INSTALL lib/orocos${OROCOS_SUFFIX}/plugins )
orocos_service( os OSService.cpp INSTALL lib/orocos${OROCOS_SUFFIX}/plugins )
orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )

     
