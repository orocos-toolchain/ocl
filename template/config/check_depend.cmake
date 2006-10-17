###########################################################
#                                                         #
# Look for dependencies required by individual components #
#                                                         #
###########################################################

# Can we use pkg-config?
INCLUDE (${PROJ_SOURCE_DIR}/config/FindPkgConfig.cmake)

# Look for boost
CHECK_INCLUDE_FILE_CXX( boost/numeric/ublas/matrix.hpp BOOST )
IF ( BOOST )
    MESSAGE("-- Looking for Boost - found")
ELSE ( BOOST )
    MESSAGE("-- Looking for Boost - not found")
ENDIF ( BOOST )

# Look for ulapack
CHECK_INCLUDE_FILE_CXX( ulapack/eig.hpp ULAPACK )


