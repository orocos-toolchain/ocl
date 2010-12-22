OPTION( BUILD_REPORTING "Build Reporting Component" ON )
# Set to OFF by default as it does not compile
OPTION( BUILD_REPORTING_NETCDF "Build the Netcdf Reporting Component" OFF )

IF ( BUILD_REPORTING )

    find_package(RTTPlugin REQUIRED rtt-marshalling)

    # This gathers all the .cpp files into the variable 'SRCS'
    SET( SRCS ConsoleReporting.cpp FileReporting.cpp ReportingComponent.cpp )
    SET( HPPS ConsoleReporting.hpp  FileReporting.hpp NiceHeaderMarshaller.hpp ReportingComponent.hpp)

    # Reporting to a socket
    SET( SOCKET_SRCS command.cpp datasender.cpp socket.cpp socketmarshaller.cpp TcpReporting.cpp)
    SET( SOCKET_HPPS command.hpp datasender.hpp socket.hpp socketmarshaller.hpp TcpReporting.hpp)

    if(NOT OROCOS_TARGET STREQUAL "win32")
      set(SRCS ${SRCS} ${SOCKET_SRCS})
      set(HPPS ${HPPS} ${SOCKET_HPPS})
    endif()

    INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )

    orocos_component( orocos-reporting ${SRCS} INSTALL lib/orocos)
    orocos_install_headers( ${HPPS} INSTALL include/orocos/ocl )

    IF ( BUILD_REPORTING_NETCDF AND NETCDF_FOUND )
      SET( NETCDF_SRCS NetcdfReporting.cpp )
      SET( NETCDF_HPPS NetcdfReporting.hpp NetcdfMarshaller.hpp NetcdfHeaderMarshaller.hpp )

      orocos_component( orocos-reporting-netcdf ${NETCDF_SRCS} INSTALL lib/orocos)
      GLOBAL_ADD_INCLUDE ( ocl ${NETCDF_HPPS} )
      target_link_libraries( orocos-reporting-netcdf ${NETCDF_LIBS} orocos-reporting )
    ENDIF ( BUILD_REPORTING_NETCDF AND NETCDF_FOUND )
    
    target_link_libraries( orocos-reporting ${RTT_PLUGIN_rtt-marshalling_LIBRARIES} )

    orocos_generate_package()
    
    ADD_SUBDIRECTORY( tests )
ENDIF ( BUILD_REPORTING )
