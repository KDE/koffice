include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(kpr_pageeffect_iriswipe_SRCS
    Plugin.cpp
    SCIrisWipeEffectStrategyBase.cpp

    irisWipe/SCIrisWipeEffectFactory.cpp
    triangleWipe/SCTriangleWipeEffectFactory.cpp
    arrowHeadWipe/SCArrowHeadWipeEffectFactory.cpp
    ellipseWipe/SCEllipseWipeEffectFactory.cpp
    roundRectWipe/SCRoundRectWipeEffectFactory.cpp
    pentagonWipe/SCPentagonWipeEffectFactory.cpp
    hexagonWipe/SCHexagonWipeEffectFactory.cpp
    starWipe/SCStarWipeEffectFactory.cpp
    eyeWipe/SCEyeWipeEffectFactory.cpp
    miscShapeWipe/SCMiscShapeWipeEffectFactory.cpp
    )

kde4_add_plugin(kpr_pageeffect_iriswipe ${kpr_pageeffect_iriswipe_SRCS})

target_link_libraries(
    kpr_pageeffect_iriswipe
    showcaseprivate
    flake
    
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}    
    )

install(TARGETS kpr_pageeffect_iriswipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kpr_pageeffect_iriswipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


