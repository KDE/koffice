
include_directories( ${CMAKE_SOURCE_DIR}/showcase ${CMAKE_SOURCE_DIR}/lib/kotext ${CMAKE_SOURCE_DIR}/filters/libdialogfilter )


########### next target ###############

set(showcasegenericimageexport_PART_SRCS genericimageexport.cpp ${libdialogfilter_SRCS})

kde4_add_plugin(showcasegenericimageexport ${showcasegenericimageexport_PART_SRCS})

target_link_libraries(showcasegenericimageexport  ${KDE4_KDECORE_LIBS} showcaseprivate )

install(TARGETS showcasegenericimageexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  showcase_bmp_export.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  showcase_jpeg_export.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  showcase_mng_export.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  showcase_png_export.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  showcase_xbm_export.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  showcase_xpm_export.desktop DESTINATION ${SERVICES_INSTALL_DIR})

