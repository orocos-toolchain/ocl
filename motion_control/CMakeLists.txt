DEPENDENT_OPTION( BUILD_MOTION "Build Motion Control Components" ON "KDL_FOUND" OFF )

# Always process the subdirs, they check their own dependent_option self.
ADD_SUBDIRECTORY( naxes )
ADD_SUBDIRECTORY( cartesian )

#IF ( NOT BUILD_MOTION )
#    MESSAGE( "**** KDL is not present, thus the Motion Control components can not be built ! ****" )
#ENDIF ( NOT BUILD_MOTION )
