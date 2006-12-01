OPTION(DOC_BUILD_DOCS "Build the documentation" ON)

include(DocBookToHTML.cmake)
include(DocBookToPDF.cmake)

IF (DOC_BUILD_DOCS)
  #list the xml files:
  DOCBOOK_TO_HTML( orocos-html.xsl 
    CATALOG
    catalog.xml
    FILES
    orocos-hardware-components.xml
    orocos-taskbrowser.xml)

  DOCBOOK_TO_PDF( orocos-pdf.xsl 
    CATALOG
    catalog.xml
    FILES
    orocos-hardware-components.xml
    orocos-taskbrowser.xml)

  add_custom_command(OUTPUT taskbrowsing.png
    COMMAND inkscape -z -D -d 60 -e taskbrowsing.png ${CMAKE_CURRENT_SOURCE_DIR}/taskbrowsing.svg
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/taskbrowsing.svg
    )

  ADD_CUSTOM_TARGET(dochtml DEPENDS taskbrowsing.png orocos-hardware-components.html orocos-taskbrowser.html)
  ADD_CUSTOM_TARGET(docpdf DEPENDS taskbrowsing.png orocos-hardware-components.pdf orocos-taskbrowser.pdf)

  ADD_CUSTOM_TARGET(message ALL echo "Use: make dochtml docpdf to build the documentation.")

ENDIF (DOC_BUILD_DOCS)
