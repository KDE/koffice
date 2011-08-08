

include_directories( ${CMAKE_SOURCE_DIR}/artwork ${CMAKE_SOURCE_DIR}/artwork/core  ${KDE4_INCLUDES}  )


########### next target ###############

set(artworkpngexport_PART_SRCS PngExport.cpp PngExportOptionsWidget.cpp )

kde4_add_ui_files( artworkpngexport_PART_SRCS PngExportOptionsWidget.ui )

kde4_add_plugin(artworkpngexport ${artworkpngexport_PART_SRCS})

target_link_libraries(artworkpngexport artworkui komain)

install(TARGETS artworkpngexport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  artwork_png_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
