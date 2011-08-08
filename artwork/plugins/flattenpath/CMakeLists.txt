

include_directories( ${CMAKE_SOURCE_DIR}/artwork ${CMAKE_SOURCE_DIR}/artwork/core ${KDE4_INCLUDES}  )


########### next target ###############

set(artwork_flattenpathplugin_PART_SRCS FlattenPathPlugin.cpp ArtworkPathFlattenCommand.cpp )


kde4_add_plugin(artwork_flattenpathplugin ${artwork_flattenpathplugin_PART_SRCS})



target_link_libraries(artwork_flattenpathplugin artworkcommon)

install(TARGETS artwork_flattenpathplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  FlattenPathPlugin.rc  DESTINATION ${DATA_INSTALL_DIR}/artwork/kpartplugins)


