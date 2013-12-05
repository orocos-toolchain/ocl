OPTION( BUILD_REPORTING "Build Reporting Component" ON )
OPTION( BUILD_REPORTING_NETCDF "Build the Netcdf Reporting Component" ON )

IF ( BUILD_REPORTING )

    find_package(RTTPlugin REQUIRED rtt-marshalling)

    # This gathers all the .cpp files into the variable 'SRCS'
    SET( SRCS ConsoleReporting.cpp FileReporting.cpp ReportingComponent.cpp )
    SET( HPPS ConsoleReporting.hpp  FileReporting.hpp NiceHeaderMarshaller.hpp ReportingComponent.hpp TableMarshaller.hpp)

    # Reporting to a socket
    SET( SOCKET_SRCS command.cpp datasender.cpp socket.cpp socketmarshaller.cpp TcpReporting.cpp)
    SET( SOCKET_HPPS command.hpp datasender.hpp socket.hpp socketmarshaller.hpp TcpReporting.hpp)

    if(NOT OROCOS_TARGET STREQUAL "win32")
      set(SRCS ${SRCS} ${SOCKET_SRCS})
      set(HPPS ${HPPS} ${SOCKET_HPPS})
    endif()

    INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

    orocos_component( orocos-ocl-reporting ${SRCS} )
    SET_TARGET_PROPERTIES( orocos-ocl-reporting PROPERTIES
      DEFINE_SYMBOL OCL_DLL_EXPORT)

    orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )

    IF ( BUILD_REPORTING_NETCDF AND NETCDF_FOUND )
      SET( NETCDF_SRCS NetcdfReporting.cpp )
      SET( NETCDF_HPPS NetcdfReporting.hpp NetcdfMarshaller.hpp NetcdfHeaderMarshaller.hpp )

      orocos_component( orocos-ocl-reporting-netcdf ${NETCDF_SRCS} )
      orocos_install_headers( ${NETCDF_HPPS} INSTALL include/orocos/ocl )
      SET_TARGET_PROPERTIES( orocos-ocl-reporting-netcdf PROPERTIES
        DEFINE_SYMBOL OCL_DLL_EXPORT)
      target_link_libraries( orocos-ocl-reporting-netcdf ${NETCDF_LIBS} orocos-ocl-reporting )
    ENDIF ( BUILD_REPORTING_NETCDF AND NETCDF_FOUND )
    
    target_link_libraries( orocos-ocl-reporting ${RTT_PLUGIN_rtt-marshalling_${OROCOS_TARGET}_LIBRARIES} )

    orocos_generate_package()
    
    ADD_SUBDIRECTORY( tests )
ENDIF ( BUILD_REPORTING )
