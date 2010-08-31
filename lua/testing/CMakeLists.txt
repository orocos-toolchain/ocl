cmake_dependent_option( BUILD_LUA_TESTCOMP "Build test-component used for testing lua bindings" OFF "BUILD_LUA_COMPONENT" OFF )

if ( BUILD_LUA_TESTCOMP )

    file( GLOB SRCS [^.]*.cpp )
    file( GLOB HPPS [^.]*.hpp )

    global_add_component( testcomp ${SRCS} )
    target_link_libraries( testcomp ${OROCOS_RTT_LIBS} )
    
endif ( BUILD_LUA_TESTCOMP )
