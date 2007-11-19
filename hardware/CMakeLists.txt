OPTION( BUILD_HARDWARE "Build Hardware Components" OFF )

    # IMPORTANT: The order must be the same as the dependencies:

    ADD_SUBDIRECTORY( demotool )
    ADD_SUBDIRECTORY( camera )
    ADD_SUBDIRECTORY( krypton )
    ADD_SUBDIRECTORY( laserdistance )
    ADD_SUBDIRECTORY( laserscanner )
    ADD_SUBDIRECTORY( wrench )
    ADD_SUBDIRECTORY( can )
    ADD_SUBDIRECTORY( beckhoff )
    ADD_SUBDIRECTORY( apci )
    ADD_SUBDIRECTORY( comedi )
    ADD_SUBDIRECTORY( io )
    ADD_SUBDIRECTORY( axes )
    ADD_SUBDIRECTORY( kuka )
    ADD_SUBDIRECTORY( lias )
    ADD_SUBDIRECTORY( robotsimulation )
    ADD_SUBDIRECTORY( staubli )
#    ADD_SUBDIRECTORY( performer_mk2 )
    