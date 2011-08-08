
include_directories( ${CMAKE_SOURCE_DIR}/artwork
${CMAKE_SOURCE_DIR}/artwork/core ${CMAKE_SOURCE_DIR}/plugins/pathshapes  ${KDE4_INCLUDES}  )

########### next target ###############

set(artworksvgexport_PART_SRCS 
    svgexport.cc
    SvgUtil.cpp
    ${SHAPES_SOURCES} 
)

kde4_add_plugin(artworksvgexport ${artworksvgexport_PART_SRCS})

target_link_libraries(artworksvgexport artworkui)

install(TARGETS artworksvgexport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(artworksvgimport_PART_SRCS 
    svgimport.cc
    SvgUtil.cpp
    SvgParser.cpp
    SvgGradientHelper.cpp
    SvgPatternHelper.cpp
    SvgFilterHelper.cpp
    SvgCssHelper.cpp
    ${SHAPES_SOURCES} 
)

kde4_add_plugin(artworksvgimport ${artworksvgimport_PART_SRCS})

target_link_libraries(artworksvgimport artworkui)

install(TARGETS artworksvgimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  artwork_svg_export.desktop artwork_svg_import.desktop artwork_svgz_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
