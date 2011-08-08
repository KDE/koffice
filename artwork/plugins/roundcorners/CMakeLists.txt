

include_directories( ${CMAKE_SOURCE_DIR}/artwork ${CMAKE_SOURCE_DIR}/artwork/core ${KDE4_INCLUDES}  )


########### next target ###############

set(artwork_roundcornersplugin_PART_SRCS RoundCornersPlugin.cpp RoundCornersCommand.cpp )


kde4_add_plugin(artwork_roundcornersplugin ${artwork_roundcornersplugin_PART_SRCS})



target_link_libraries(artwork_roundcornersplugin artworkcommon)

install(TARGETS artwork_roundcornersplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  RoundCornersPlugin.rc  DESTINATION ${DATA_INSTALL_DIR}/artwork/kpartplugins)





