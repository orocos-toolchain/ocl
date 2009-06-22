OPTION(DOC_BUILD_DOCS "Build the documentation" ON)

include(DocBookToHTML.cmake)
include(DocBookToPDF.cmake)
include(AnyToPNG.cmake)

IF (DOC_BUILD_DOCS)
  #list the xml files:
  SET( XML_SOURCES
    xml/orocos-deployment.xml xml/orocos-reporting.xml xml/orocos-taskbrowser.xml xml/orocos-hardware-components.xml
    xml/ocl-overview.xml xml/orocos-naxes.xml xml/orocos-comedi.xml)

  SEPARATE_ARGUMENTS(XML_SOURCES)

  DOCBOOK_TO_HTML( HTML_OUTPUT orocos-html.xsl 
    CATALOG
    xml/catalog.xml
    FILES 
    ${XML_SOURCES} )
  #MESSAGE( "HTML_OUTPUT: ${HTML_OUTPUT}")

  DOCBOOK_TO_PDF( PDF_OUTPUT orocos-pdf.xsl
    CATALOG
    xml/catalog.xml
    FILES
    ${XML_SOURCES} )

  FILE( GLOB PNGS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} xml/images/[^.]*.png )
  FILE( GLOB HR_PNGS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} xml/images/hires/[^.]*.png )
  FILE( GLOB DIAS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} xml/images/[^.]*.dia )
  FILE( GLOB SVGS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} xml/images/[^.]*.svg )
  FILE( GLOB ICONS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} xml/images/icons/[^.]*.png )
  #MESSAGE( "PNGS: ${PNGS}, path: ${CMAKE_CURRENT_SOURCE_DIR}/xml/images" )
  ANY_TO_PNG( RESULT_PNGS ${HR_PNGS} ${PNGS} ${DIAS} ${SVGS} ${ICONS} )

  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/xml/orocos-html.css
	 COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/orocos-html.css ${CMAKE_CURRENT_BINARY_DIR}/xml/orocos-html.css
	 DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/orocos-html.css
	 )

  ADD_CUSTOM_TARGET(docpng DEPENDS ${RESULT_PNGS})
  ADD_CUSTOM_TARGET(dochtml DEPENDS ${HTML_OUTPUT} ${CMAKE_CURRENT_BINARY_DIR}/xml/orocos-html.css)
  ADD_CUSTOM_TARGET(docpdf DEPENDS ${PDF_OUTPUT})
  ADD_DEPENDENCIES(dochtml docpng)
  ADD_DEPENDENCIES(docpdf docpng)

  ADD_CUSTOM_TARGET(message ALL echo "Use: 'make docapi dochtml docpdf' to build the documentation.")

ENDIF (DOC_BUILD_DOCS)
