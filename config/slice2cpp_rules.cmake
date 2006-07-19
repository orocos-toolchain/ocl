#
# This file contains a set of macros used for generating 
# C++ source from SLICE files
#
# Author: Alex Brooks
#

SET ( SLICE_CPP_SUFFIXES     .cpp )
SET ( SLICE_HEADER_SUFFIXES  .h  ) 
SET ( SLICE_SUFFIXES         ${SLICE_CPP_SUFFIXES} ${SLICE_HEADER_SUFFIXES} ) 

SET ( SLICE_EXE ${ICE_HOME}/bin/slice2cpp )

SET ( SLICE_SOURCE_DIR ${PROJ_SOURCE_DIR}/src/interfaces/slice )
SET ( SLICE_BINARY_DIR ${PROJ_BINARY_DIR}/src/interfaces/slice )
SET ( SLICE2CPP_BINARY_DIR ${PROJ_BINARY_DIR}/src/interfaces/cpp )

SET ( SLICE_ARGS ${SLICE_PROJECT_ARGS} -I${SLICE_SOURCE_DIR} -I${ICE_HOME}/slice --stream --output-dir ${SLICE2CPP_BINARY_DIR}/orca )

#
# Appends the new_bit to the original.
# If the original is not set, it will be set to the new_bit.
#
MACRO( APPEND ORIGINAL NEW_BIT )

  IF    ( NOT ${ORIGINAL} )
    SET( ${ORIGINAL} ${NEW_BIT} )
  ELSE  ( NOT ${ORIGINAL} )
    SET( ${ORIGINAL} ${${ORIGINAL}} ${NEW_BIT} )
  ENDIF ( NOT ${ORIGINAL} )

ENDMACRO( APPEND ORIGINAL NEW_BIT )

#
# Generate rules for SLICE->C++ files generation, for each
# of the named slice source files.
#
# Usage: GENERATE_SLICE2CPP_RULES( GENERATED_CPP_LIST GENERATED_HEADER_LIST [ SRC1 [ SRC2 ... ] ] )
# 
# Returns lists of all the .cpp and .h files that will be generated.
#
MACRO ( GENERATE_SLICE2CPP_RULES GENERATED_CPP_LIST GENERATED_HEADER_LIST )

  MESSAGE ( "Will generate cpp header and source files from slice definitions" )
  MESSAGE ( "   slice sources    : " ${SLICE_SOURCE_DIR} )
  MESSAGE ( "   cpp destination  : " ${SLICE2CPP_BINARY_DIR} )
  MESSAGE ( "   sample command   : " ${SLICE_EXE} " <source.ice> " ${SLICE_ARGS} )

  # debug
  #MESSAGE( "GENERATE_SLICE2CPP_RULES: ARGN: ${ARGN}" )

  #
  # First pass: Loop through the SLICE sources we were given, add the full path for dependencies
  #
  FOREACH( SLICE_SOURCE_BASENAME ${ARGN} )
    APPEND( GEN_SLICE_RULES_SLICE_DEPENDS "${SLICE_SOURCE_DIR}/orca/${SLICE_SOURCE_BASENAME}" )
  ENDFOREACH( SLICE_SOURCE_BASENAME ${ARGN} )

  #
  # Second pass: Loop through the SLICE sources we were given, add the CMake rules
  #
  FOREACH( SLICE_SOURCE_BASENAME ${ARGN} )
     SET( SLICE_SOURCE "${SLICE_SOURCE_DIR}/orca/${SLICE_SOURCE_BASENAME}" )

      #MESSAGE("Dealing with ${SLICE_SOURCE_BASENAME}")

    #
    # Add a custom cmake command to generate each type of output file: headers and source
    #
    FOREACH ( SUFFIX ${SLICE_SUFFIXES} )

      # OUTPUT is the target we're generating rules for.
      STRING( REGEX REPLACE "\\.ice" ${SUFFIX} OUTPUT_BASENAME "${SLICE_SOURCE_BASENAME}" )
      SET( OUTPUT_FULLNAME "${SLICE2CPP_BINARY_DIR}/orca/${OUTPUT_BASENAME}" )

      #
      # Make each .h and .cpp file depend on _every_ slice source.  This means that if you 
      # change any .ice file everything will be recompiled.  This is done because CMake can't 
      # track dependencies between .ice files.
      #
      SET( DEPENDS ${GEN_SLICE_RULES_SLICE_DEPENDS} )

      #
      # Let CMake know that it has to generate the .h file before compiling the .cpp
      #
      IF( ${SUFFIX} STREQUAL ".cpp" )
        STRING( REGEX REPLACE "\\.cpp" ".h" ASSOCIATED_HEADER "${OUTPUT_FULLNAME}" )
        SET( DEPENDS ${DEPENDS} ${ASSOCIATED_HEADER} )
      ENDIF( ${SUFFIX} STREQUAL ".cpp" )

      #
      # Add the command to generate file.xxx from file.ice
      # Note: when the 'output' is needed, the 'command' will be called with the 'args'
      #
      MESSAGE ( "Adding rule for generating ${OUTPUT_BASENAME} from ${SLICE_SOURCE_BASENAME}" )
      ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_BASENAME}
        COMMAND ${SLICE_EXE} 
        ARGS ${SLICE_SOURCE} ${SLICE_ARGS}
        DEPENDS ${DEPENDS}
        )
        
      #
      # Add this output to the list of generated files
      #
      IF( ${SUFFIX} STREQUAL ".cpp" )
        APPEND( ${GENERATED_CPP_LIST} ${OUTPUT_BASENAME} )
      ELSE( ${SUFFIX} STREQUAL ".cpp" )
        APPEND( ${GENERATED_HEADER_LIST} ${OUTPUT_BASENAME} )
      ENDIF( ${SUFFIX} STREQUAL ".cpp" )

    ENDFOREACH ( SUFFIX ${SLICE_SUFFIXES} )

  ENDFOREACH( SLICE_SOURCE_BASENAME ${ARGN} )

  #
  # Tell CMake that these files are generated, and need to be deleted on 'make clean'.
  # This also prevents cmake from bitching when we add non-existent files to targets.
  #
  SET_SOURCE_FILES_PROPERTIES(${${GENERATED_HEADER_LIST}} PROPERTIES GENERATED true)
  SET_SOURCE_FILES_PROPERTIES(${${GENERATED_CPP_LIST}}    PROPERTIES GENERATED true)
  SET_DIRECTORY_PROPERTIES( 
    PROPERTIES 
    ADDITIONAL_MAKE_CLEAN_FILES 
    "${${GENERATED_HEADER_LIST}} ${${GENERATED_CPP_LIST}}" )

  #MESSAGE("GENERATED_CPP_LIST: ${${GENERATED_CPP_LIST}}")

ENDMACRO ( GENERATE_SLICE2CPP_RULES GENERATED_CPP_LIST GENERATED_HEADER_LIST )
