
include_directories( ${POPPLER_INCLUDE_DIR} )

macro_optional_find_package(OpenJPEG)
macro_optional_find_package(JPEG)

set(artworkpdfimport_PART_SRCS PdfImport.cpp SvgOutputDev.cpp )

kde4_add_plugin(artworkpdfimport ${artworkpdfimport_PART_SRCS})

target_link_libraries(artworkpdfimport komain poppler poppler-qt4)
if(JPEG_FOUND)
    target_link_libraries(artworkpdfimport jpeg)
endif(JPEG_FOUND)
if(OPENJPEG_FOUND)
    target_link_libraries(artworkpdfimport ${OPENJPEG_LIBRARIES})
endif(OPENJPEG_FOUND)

install(TARGETS artworkpdfimport DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES artwork_pdf_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
