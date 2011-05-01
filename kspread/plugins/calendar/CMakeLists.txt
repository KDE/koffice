include_directories(${CMAKE_SOURCE_DIR}/kspread ${KOMAIN_INCLUDES})

set(kcells_plugin_tool_calendar_SRCS CalendarTool.cpp CalendarToolFactory.cpp CalendarToolWidget.cpp)
kde4_add_ui_files(kcells_plugin_tool_calendar_SRCS CalendarToolWidget.ui)
kde4_add_plugin(kcells_plugin_tool_calendar ${kcells_plugin_tool_calendar_SRCS})
target_link_libraries(kcells_plugin_tool_calendar kcellscommon)
install(TARGETS kcells_plugin_tool_calendar DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcells_plugin_tool_calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR})
