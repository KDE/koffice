include_directories( ${KOMAIN_INCLUDES} ${KOPAGEAPP_INCLUDES} ${CMAKE_SOURCE_DIR}/showcase/part ${Boost_INCLUDE_DIR} )

set(showcasevariables_PART_SRCS
    PresentationVariable.cpp
    PresentationVariableFactory.cpp
    Plugin.cpp
   )

kde4_add_plugin(showcasevariables ${showcasevariables_PART_SRCS})

target_link_libraries(
    showcasevariables
    showcaseprivate
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    )

install(TARGETS showcasevariables  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcasevariables.desktop DESTINATION ${SERVICES_INSTALL_DIR})
