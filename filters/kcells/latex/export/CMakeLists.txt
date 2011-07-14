add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories(${CMAKE_SOURCE_DIR}/kcells ${KOMAIN_INCLUDES})

########### next target ###############

set(kcellslatexexport_PART_SRCS 
   cell.cc 
   column.cc 
   config.cc 
   document.cc 
   fileheader.cc 
   format.cc 
   kcellslatexexportdiaImpl.cc 
   latexexport.cc 
   latexexportAdaptor.cc 
   map.cc 
   pen.cc 
   row.cc 
   spreadsheet.cc 
   table.cc 
   xmlparser.cc )

kde4_add_ui_files(kcellslatexexport_PART_SRCS latexexportdia.ui )

kde4_add_plugin(kcellslatexexport ${kcellslatexexport_PART_SRCS})



target_link_libraries(kcellslatexexport komain ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcellslatexexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kcells_latex_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})



