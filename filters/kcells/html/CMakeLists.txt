include_directories( ${CMAKE_SOURCE_DIR}/kcells ${KDE4_INCLUDES} ${KOMAIN_INCLUDES} )

set(kcellshtmlimport_PART_SRCS htmlimport.cc )
kde4_add_plugin(kcellshtmlimport ${kcellshtmlimport_PART_SRCS})
target_link_libraries(kcellshtmlimport komain ${KDE4_KHTML_LIBS})
install(TARGETS kcellshtmlimport  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kcells_html_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

set(kcellshtmlexport_PART_SRCS htmlexport.cc exportdialog.cc )
kde4_add_ui_files(kcellshtmlexport_PART_SRCS exportwidget.ui )
kde4_add_plugin(kcellshtmlexport ${kcellshtmlexport_PART_SRCS})
target_link_libraries(kcellshtmlexport kcellscommon)
install(TARGETS kcellshtmlexport  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kcells_html_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

