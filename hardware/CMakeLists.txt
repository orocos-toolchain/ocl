OPTION( BUILD_HARDWARE "Build Hardware Components" ON )

IF ( BUILD_HARDWARE )

    ADD_SUBDIRECTORY( demotool )
    ADD_SUBDIRECTORY( kuka )
    ADD_SUBDIRECTORY( lias )
    ADD_SUBDIRECTORY( camera )
    ADD_SUBDIRECTORY( krypton )
    ADD_SUBDIRECTORY( laserdistance )
    ADD_SUBDIRECTORY( wrench )

ENDIF ( BUILD_HARDWARE )
