OPTION( BUILD_HARDWARE "Build Hardware Components" OFF )

IF ( BUILD_HARDWARE )

    ADD_SUBDIRECTORY( demotool )
    ADD_SUBDIRECTORY( kuka )
    ADD_SUBDIRECTORY( lias )
    ADD_SUBDIRECTORY( camera )
    ADD_SUBDIRECTORY( krypton )
    ADD_SUBDIRECTORY( laserdistance )
    ADD_SUBDIRECTORY( wrench )
    ADD_SUBDIRECTORY( canpie )
    ADD_SUBDIRECTORY( beckhoff )
    ADD_SUBDIRECTORY( apci )
    ADD_SUBDIRECTORY( comedi )
    ADD_SUBDIRECTORY( io )

ENDIF ( BUILD_HARDWARE )
