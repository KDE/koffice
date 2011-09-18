include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(showcase_pageeffect_iriswipe_SRCS
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

kde4_add_plugin(showcase_pageeffect_iriswipe ${showcase_pageeffect_iriswipe_SRCS})

target_link_libraries(showcase_pageeffect_iriswipe showcaseprivate)

install(TARGETS showcase_pageeffect_iriswipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_iriswipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


