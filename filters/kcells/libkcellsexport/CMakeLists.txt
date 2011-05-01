

include_directories( ${CMAKE_SOURCE_DIR}/kcells ${KDE4_INCLUDES}  )


########### next target ###############

set(kcellsexport_LIB_SRCS KCellsLeader.cc KCellsBaseWorker.cc )


kde4_add_library(kcellsexport SHARED ${kcellsexport_LIB_SRCS})

target_link_libraries(kcellsexport  ${KDE4_KDECORE_LIBS} )

set_target_properties(kcellsexport PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kcellsexport  ${INSTALL_TARGETS_DEFAULT_ARGS})
