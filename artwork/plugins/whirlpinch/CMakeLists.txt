
include_directories( ${CMAKE_SOURCE_DIR}/artwork ${CMAKE_SOURCE_DIR}/artwork/core ${KDE4_INCLUDES}  )

########### next target ###############

set(artwork_whirlpinchplugin_PART_SRCS WhirlPinchPlugin.cpp ArtworkWhirlPinchCommand.cpp )

kde4_add_plugin(artwork_whirlpinchplugin ${artwork_whirlpinchplugin_PART_SRCS})

target_link_libraries(artwork_whirlpinchplugin artworkcommon)

install(TARGETS artwork_whirlpinchplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  WhirlPinchPlugin.rc  DESTINATION ${DATA_INSTALL_DIR}/artwork/kpartplugins)

