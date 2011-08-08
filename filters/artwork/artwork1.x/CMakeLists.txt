
include_directories( ${CMAKE_SOURCE_DIR}/artwork
${CMAKE_SOURCE_DIR}/artwork/core ${CMAKE_SOURCE_DIR}/shapes/ ${KDE4_INCLUDES}  )

########### next target ###############

set(artwork1ximport_PART_SRCS 
    Artwork1xImport.cpp
    ${SHAPES_SOURCES} )

kde4_add_plugin(artwork1ximport ${artwork1ximport_PART_SRCS})

target_link_libraries(artwork1ximport artworkcommon artworkui)

install(TARGETS artwork1ximport DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES artwork_1x_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
