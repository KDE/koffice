include_directories(${CMAKE_SOURCE_DIR}/kcells ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${EIGEN2_INCLUDE_DIR})


########### next target ###############

kde4_add_plugin(kcellsbitopsmodule bitops.cpp)
target_link_libraries(kcellsbitopsmodule kcellscommon)
install(TARGETS kcellsbitopsmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsbitopsmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES bitops.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsconversionmodule conversion.cpp)
target_link_libraries(kcellsconversionmodule kcellscommon)
install(TARGETS kcellsconversionmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsconversionmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES conversion.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsdatabasemodule database.cpp)
target_link_libraries(kcellsdatabasemodule kcellscommon)
install(TARGETS kcellsdatabasemodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsdatabasemodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES database.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsdatetimemodule datetime.cpp)
target_link_libraries(kcellsdatetimemodule kcellscommon)
install(TARGETS kcellsdatetimemodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsdatetimemodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES datetime.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsengineeringmodule engineering.cpp)
target_link_libraries(kcellsengineeringmodule kcellscommon)
install(TARGETS kcellsengineeringmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsengineeringmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES engineering.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsfinancialmodule financial.cpp)
target_link_libraries(kcellsfinancialmodule kcellscommon)
install(TARGETS kcellsfinancialmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsfinancialmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES financial.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsinformationmodule information.cpp)
target_link_libraries(kcellsinformationmodule kcellscommon)
install(TARGETS kcellsinformationmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsinformationmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES information.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellslogicmodule logic.cpp)
target_link_libraries(kcellslogicmodule kcellscommon)
install(TARGETS kcellslogicmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellslogicmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES logic.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsmathmodule math.cpp)
target_link_libraries(kcellsmathmodule kcellscommon)
install(TARGETS kcellsmathmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsmathmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES math.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsreferencemodule reference.cpp)
target_link_libraries(kcellsreferencemodule kcellscommon)
install(TARGETS kcellsreferencemodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsreferencemodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES reference.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellsstatisticalmodule statistical.cpp)
target_link_libraries(kcellsstatisticalmodule kcellscommon)
install(TARGETS kcellsstatisticalmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellsstatisticalmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES statistical.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellstextmodule text.cpp)
target_link_libraries(kcellstextmodule kcellscommon)
install(TARGETS kcellstextmodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellstextmodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES text.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)

########### next target ###############

kde4_add_plugin(kcellstrigonometrymodule trig.cpp)
target_link_libraries(kcellstrigonometrymodule kcellscommon)
install(TARGETS kcellstrigonometrymodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcellstrigonometrymodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES trig.xml DESTINATION ${DATA_INSTALL_DIR}/kcells/functions)
