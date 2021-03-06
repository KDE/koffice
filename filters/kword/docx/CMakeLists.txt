add_definitions(-DKDE_DEFAULT_DEBUG_AREA=30526)

include_directories(
    ${CMAKE_SOURCE_DIR}/filters/libmsooxml
    ${CMAKE_SOURCE_DIR}/filters/kcells/xlsx # for Charting
)

########### next target ###############

set(docximport_PART_SRCS
    DocxXmlStylesReader.cpp
    DocxXmlFontTableReader.cpp
    DocxXmlCommentsReader.cpp
    DocxXmlNotesReader.cpp
    DocxXmlDocumentReader.cpp
    DocxXmlHeaderReader.cpp
    DocxXmlFooterReader.cpp
    DocxXmlNumberingReader.cpp
    DocxXmlFootnoteReader.cpp
    DocxXmlEndnoteReader.cpp
    DocxImport.cpp
    ${CMAKE_SOURCE_DIR}/filters/kcells/xlsx/ChartExport.cpp
    ${CMAKE_SOURCE_DIR}/filters/kcells/xlsx/XlsxXmlChartReader.cpp
)

kde4_add_plugin(docximport ${docximport_PART_SRCS})

target_link_libraries(
    docximport
    msooxml
    komain
)

install(TARGETS docximport DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

IF(FALSE) # not implemented yet

set(docxexport_PART_SRCS DocxExport.cpp ExportFilter.cpp ${libcommon_SRCS} ${liboofilter_SRCS})

kde4_add_plugin(docxexport ${docxexport_PART_SRCS})



target_link_libraries(
    docxexport
    msooxml
    komain
)

install(TARGETS docxexport DESTINATION ${PLUGIN_INSTALL_DIR})

ENDIF(FALSE)

########### next target ###############

########### install files ###############

install( FILES 
  kword_docx_import.desktop
#todo  kword_docx_export.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)
