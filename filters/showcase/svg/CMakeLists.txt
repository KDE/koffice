

include_directories( ${CMAKE_SOURCE_DIR}/showcase ${CMAKE_SOURCE_DIR}/lib/kotext ${KDE4_INCLUDES}  )


########### next target ###############

set(showcasesvgexport_PART_SRCS svgexport.cc )


kde4_add_plugin(showcasesvgexport ${showcasesvgexport_PART_SRCS})



target_link_libraries(showcasesvgexport  ${KDE4_KDECORE_LIBS} showcaseprivate )

install(TARGETS showcasesvgexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  showcase_svg_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


