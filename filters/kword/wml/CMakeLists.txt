


add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)


########### next target ###############

set(wmlimport_PART_SRCS wmlimport.cc wmlparser.cpp )


kde4_add_plugin(wmlimport ${wmlimport_PART_SRCS})



target_link_libraries(wmlimport kwordexportfilters)

install(TARGETS wmlimport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(wmlexport_PART_SRCS wmlexport.cc )

kde4_add_plugin(wmlexport ${wmlexport_PART_SRCS})



target_link_libraries(wmlexport kwordexportfilters)

install(TARGETS wmlexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kword_wml_import.desktop kword_wml_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


