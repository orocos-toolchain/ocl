#
# Include and link against liborocos-rtt and its dependencies
#
INCLUDE_DIRECTORIES( 
    ${PROJ_SOURCE_DIR}/src/utils
)

LINK_DIRECTORIES(
    ${PROJ_BINARY_DIR}/src/utils/orcaice
    ${PROJ_BINARY_DIR}/src/utils/orcaobj
)

LINK_LIBRARIES( OrcaIce OrcaObjects )
