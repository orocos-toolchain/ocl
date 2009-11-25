
DEPENDENT_OPTION( BUILD_REPORTING_TEST "Build Reporting Test" ON "BUILD_REPORTING;BUILD_TESTS" OFF)

IF ( BUILD_REPORTING_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'

    GLOBAL_ADD_TEST( report main.cpp )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( report orocos-taskbrowser orocos-reporting )

    GLOBAL_ADD_TEST( tcpreport tcpmain.cpp )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( tcpreport orocos-taskbrowser orocos-reporting )

    # Copy this file to build dir.
    TEST_USES_FILE( reporter.cpf )

  IF ( NETCDF_FOUND )

    GLOBAL_ADD_TEST( ncreport ncmain.cpp )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( ncreport orocos-taskbrowser orocos-reporting-netcdf )

  ENDIF( NETCDF_FOUND )

ENDIF ( BUILD_REPORTING_TEST )

