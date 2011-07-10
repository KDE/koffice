
include_directories( ${KOMAIN_INCLUDES} )


########### next target ###############

set(kofficescan_PART_SRCS scan.cpp )


kde4_add_plugin(kofficescan ${kofficescan_PART_SRCS})

target_link_libraries(kofficescan komain )

install(TARGETS kofficescan  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  scan-showcase.rc  DESTINATION ${DATA_INSTALL_DIR}/showcase/kpartplugins)
install( FILES  scan-kword.rc  DESTINATION ${DATA_INSTALL_DIR}/kword/kpartplugins)


