OPTION(BUILD_DOCS "Build the documentation" ON)

include(DocBookToHTML.cmake)
include(DocBookToPDF.cmake)

IF (BUILD_DOCS)
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

  ADD_CUSTOM_TARGET(dochtml DEPENDS orocos-hardware-components.html orocos-taskbrowser.html)
  ADD_CUSTOM_TARGET(docpdf DEPENDS orocos-hardware-components.pdf orocos-taskbrowser.pdf)


ENDIF (BUILD_DOCS)
