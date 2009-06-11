# Set COMEDI link and compilation flags.

# GNU/Linux and Xenomai building and linking.
IF (OS_GNULINUX OR OS_XENOMAI)
  IF( COMEDILIB_FOUND )
	INCLUDE_DIRECTORIES( ${COMEDI_INCLUDE_DIRS} )
	LINK_DIRECTORIES( ${COMEDI_LINK_DIRS} )
        LINK_LIBRARIES( ${COMEDI_LIBRARY} )
        # The user app does not need to link/include comedi stuff.
	#OROCOS_PKGCONFIG_INCPATH("${COMEDI_INCLUDE_DIRS}")
	# This ends up in Libs.private
	OROCOS_PKGCONFIG_LIBPATH("${COMEDI_INSTALL}/lib")
        OROCOS_PKGCONFIG_LIBS( comedi )
  ENDIF( COMEDILIB_FOUND )
ENDIF ( OS_GNULINUX OR OS_XENOMAI )

# For LXRT:
    IF (OS_LXRT)
	IF ( COMEDI_INCLUDE_DIR )
	  # Add comedi header path and lxrt comedi lib 
	  INCLUDE_DIRECTORIES( ${COMEDI_INSTALL}/include )
	  LINK_DIRECTORIES( ${COMEDI_LINK_DIRS} )
	  LINK_LIBRARIES( ${COMEDI_LIBRARY} pthread )
	  # The user app does not need to link/include comedi stuff.
	  #OROCOS_PKGCONFIG_INCPATH("${COMEDI_INSTALL}/include")
	  # This ends up in Libs.private
	  OROCOS_PKGCONFIG_LIBPATH("{COMEDI_INSTALL}/lib")
	  OROCOS_PKGCONFIG_LIBS( kcomedilxrt )
	ENDIF ( COMEDI_INCLUDE_DIR )
    ENDIF (OS_LXRT)

