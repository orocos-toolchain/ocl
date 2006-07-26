OPTION( BUILD_MOTION "Build Motion Control Components" ON )

IF ( BUILD_MOTION )

    ADD_SUBDIRECTORY( naxes )
    ADD_SUBDIRECTORY( cartesian )

ENDIF ( BUILD_MOTION )
