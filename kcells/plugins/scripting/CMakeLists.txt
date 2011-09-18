include_directories( 
	${CMAKE_SOURCE_DIR}/kcells
    ${CMAKE_SOURCE_DIR}/kcells/commands
	${CMAKE_SOURCE_DIR}/kcells/ui
	${KODFTEXT_INCLUDES} 
	${KOMAIN_INCLUDES} 
	${KOKROSS_INCLUDES}
	)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory( scripts )

# the KCells Scripting KParts::Plugin + Kross module
set(krossmodulekcells_PART_SRCS ScriptingModule.cpp ScriptingFunction.cpp ScriptingWidgets.cpp ScriptingPart.cpp ScriptingListener.cpp)

qt4_wrap_cpp(krossmodulekcells_PART_SRCS ScriptingReader.h ScriptingWriter.h)
kde4_add_plugin(krossmodulekcells ${krossmodulekcells_PART_SRCS})

target_link_libraries(krossmodulekcells ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} kcellscommon kokross)
install(TARGETS krossmodulekcells DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

#install( FILES  kcellsscripting.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krossmodulekcells.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  scripting.rc  DESTINATION ${DATA_INSTALL_DIR}/kcells/kpartplugins)
