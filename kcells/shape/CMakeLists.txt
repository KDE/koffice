include_directories(
	${CMAKE_SOURCE_DIR}/kcells
	${CMAKE_SOURCE_DIR}/kcells/ui
    ${KOMAIN_INCLUDES}
	${KODFTEXT_INCLUDES}
	)

########### Flake Plugin library ###############

SET (tableshape_LIB_SRCS
    TablePageManager.cpp
    TableShape.cpp
    TableShapeFactory.cpp
    )

   SET (tableshape_LIB_SRCS
       ${tableshape_LIB_SRCS}
       TableTool.cpp
       TableToolFactory.cpp
       SheetsEditor.cpp
   )

kde4_add_plugin(spreadsheetshape ${tableshape_LIB_SRCS})

target_link_libraries(spreadsheetshape kcellscommon)
install(TARGETS spreadsheetshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES spreadsheetshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
