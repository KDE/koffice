include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part)
if(NOT QT_MAC_USE_COCOA)
    include_directories(${PHONON_INCLUDE_DIR}/phonon)
else(NOT QT_MAC_USE_COCOA)
    include_directories(${PHONON_INCLUDE_DIR})
endif(NOT QT_MAC_USE_COCOA)

set(showcaseeventactions_PART_SRCS
    sound/SCSoundEventAction.cpp
    sound/SCSoundEventActionFactory.cpp
    sound/SCSoundEventActionWidget.cpp
    EventActionPlugin.cpp
)

kde4_add_plugin(showcaseeventactions ${showcaseeventactions_PART_SRCS})

target_link_libraries(
    showcaseeventactions
    showcaseprivate
    flake
    ${KDE4_KIO_LIBRARY}
    ${KDE4_PHONON_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

install(TARGETS showcaseeventactions  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES showcaseeventactions.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
