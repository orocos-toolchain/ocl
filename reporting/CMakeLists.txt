OPTION( BUILD_REPORTING "Build Reporting Component" ON )

IF ( BUILD_REPORTING )

    # This gathers all the .cpp files into the variable 'SRCS'
    SET( SRCS command.cpp ConsoleReporting.cpp datasender.cpp FileReporting.cpp ReportingComponent.cpp socket.cpp socketmarshaller.cpp TcpReporting.cpp)
    SET( HPPS command.hpp ConsoleReporting.hpp datasender.hpp FileReporting.hpp NiceHeaderMarshaller.hpp ReportingComponent.hpp socket.hpp socketmarshaller.hpp TcpReporting.hpp)

    INCLUDE_DIRECTORIES ( ${BOOST} ${BOOST_OPTIONS} )

    GLOBAL_ADD_COMPONENT( orocos-reporting ${SRCS})
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})

    IF ( NETCDF_FOUND )
      SET( NETCDF_SRCS NetcdfReporting.cpp )
      SET( NETCDF_HPPS NetcdfReporting.hpp NetcdfMarshaller.hpp NetcdfHeaderMarshaller.hpp )

      GLOBAL_ADD_COMPONENT( orocos-reporting-netcdf ${NETCDF_SRCS})
      GLOBAL_ADD_INCLUDE ( ocl ${NETCDF_HPPS} )
      COMPONENT_ADD_LIBS( orocos-reporting-netcdf ${NETCDF_LIBS} )
      COMPONENT_ADD_DEPS( orocos-reporting-netcdf orocos-reporting )
    ENDIF ( NETCDF_FOUND )
    
    ADD_SUBDIRECTORY( tests )
ENDIF ( BUILD_REPORTING )
