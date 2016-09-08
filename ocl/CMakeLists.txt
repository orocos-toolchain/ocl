
# This gathers all the .cpp files into the variable 'SRCS'
FILE( GLOB HPPS [^.]*.hpp )
FILE( GLOB SRCS [^.]*.cpp )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

orocos_component( orocos-ocl-common HMIConsoleOutput.cpp EmptyTaskContext.cpp )
SET_TARGET_PROPERTIES( orocos-ocl-common PROPERTIES
    SOVERSION ${OCL_SOVERSION}
    DEFINE_SYMBOL OCL_DLL_EXPORT)
orocos_typekit( ocl ocltoolkit.cpp )
set_target_properties(ocl PROPERTIES
    SOVERSION ${OCL_SOVERSION}
)
orocos_service( print PrintService.cpp )
set_target_properties(print PROPERTIES
    SOVERSION ${OCL_SOVERSION}
)
orocos_service( os OSService.cpp )
set_target_properties(os PROPERTIES
    SOVERSION ${OCL_SOVERSION}
)
orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )
orocos_generate_package( orocos-ocl-${OROCOS_TARGET} )
orocos_generate_package( ocl-${OROCOS_TARGET} )

     
