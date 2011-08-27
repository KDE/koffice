
include_directories( ${KWMF_INCLUDES} ${CMAKE_SOURCE_DIR}/shapes/ )

########### next target ###############

set(wmfimport_PART_SRCS 
    wmfimport.cc 
    wmfimportparser.cc
    ${SHAPES_SOURCES} )

kde4_add_plugin(wmfimport ${wmfimport_PART_SRCS})

target_link_libraries(wmfimport kowmf artworkui)

install(TARGETS wmfimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(wmfexport_PART_SRCS wmfexport.cc )

kde4_add_plugin(wmfexport ${wmfexport_PART_SRCS})

target_link_libraries(
    wmfexport
    kowmf
    artworkcommon
    artworkui
    )

install(TARGETS wmfexport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  artwork_wmf_import.desktop artwork_wmf_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

