
include_directories( ${CMAKE_SOURCE_DIR}/artwork ${CMAKE_SOURCE_DIR}/artwork/core ${KDE4_INCLUDES}  )

########### next target ###############

set(artwork_refinepathplugin_PART_SRCS RefinePathPlugin.cpp ArtworkPathRefineCommand.cpp )

kde4_add_plugin(artwork_refinepathplugin ${artwork_refinepathplugin_PART_SRCS})

target_link_libraries(artwork_refinepathplugin artworkcommon)

install(TARGETS artwork_refinepathplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES RefinePathPlugin.rc  DESTINATION ${DATA_INSTALL_DIR}/artwork/kpartplugins)
