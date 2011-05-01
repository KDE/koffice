include_directories( ${CMAKE_SOURCE_DIR}/kcells ${KDE4_INCLUDES} ${KOMAIN_INCLUDES} )
add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(gnumericexport_PART_SRCS gnumericexport.cc )
kde4_add_plugin(gnumericexport ${gnumericexport_PART_SRCS})
target_link_libraries(gnumericexport kcellscommon kowidgets)

install(TARGETS gnumericexport  DESTINATION ${PLUGIN_INSTALL_DIR})

set(gnumericimport_PART_SRCS gnumericimport.cc )
kde4_add_plugin(gnumericimport ${gnumericimport_PART_SRCS})
target_link_libraries(gnumericimport kcellscommon kowidgets)
install(TARGETS gnumericimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  kcells_gnumeric_export.desktop kcells_gnumeric_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

