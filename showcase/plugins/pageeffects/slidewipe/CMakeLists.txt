include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(showcase_pageeffect_slidewipe_PART_SRCS Plugin.cpp
SCSlideWipeEffectFactory.cpp
SCSlideWipeFromBottomStrategy.cpp
SCSlideWipeFromLeftStrategy.cpp
SCSlideWipeFromRightStrategy.cpp
SCSlideWipeFromTopStrategy.cpp
SCSlideWipeToBottomStrategy.cpp
SCSlideWipeToLeftStrategy.cpp
SCSlideWipeToRightStrategy.cpp
SCSlideWipeToTopStrategy.cpp
)


kde4_add_plugin(showcase_pageeffect_slidewipe ${showcase_pageeffect_slidewipe_PART_SRCS})

target_link_libraries(
    showcase_pageeffect_slidewipe
    showcaseprivate
    flake
    
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}    
    )

install(TARGETS showcase_pageeffect_slidewipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_slidewipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


