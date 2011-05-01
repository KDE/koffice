########### next target ###############
include_directories(${KDE4_INCLUDES} ${KOMAIN_INCLUDES})

set(applixspreadimport_PART_SRCS applixspreadimport.cc )

kde4_add_plugin(applixspreadimport ${applixspreadimport_PART_SRCS})

target_link_libraries(applixspreadimport komain kcellscommon)

install(TARGETS applixspreadimport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kcells_applixspread_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




