OPTION( BUILD_DEPLOYMENT "Build Deployment Component" ON )

IF ( BUILD_DEPLOYMENT )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB SRCS [^.]*.cpp )

    GLOBAL_ADD_COMPONENT( orocos-deployment ${SRCS} )
    GLOBAL_ADD_INCLUDE( ocl ${HPPS})
     
    ADD_SUBDIRECTORY( tests )

    # This requires the orocos-rtt library to be carefully tagged
    # with the necessary 'export' attributes, or unresolved symbols
    # will appear. Especially template classes and exceptions are 
    # sensitive.
    SET_SOURCE_FILES_PROPERTIES( ${SRCS} PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")

ENDIF ( BUILD_DEPLOYMENT )
