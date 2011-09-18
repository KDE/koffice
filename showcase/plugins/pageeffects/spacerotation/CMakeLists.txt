include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(showcase_pageeffect_spacerotation_PART_SRCS Plugin.cpp
    SCSpaceRotationEffectFactory.cpp
    SCSpaceRotationFromBottomStrategy.cpp
    SCSpaceRotationFromTopStrategy.cpp
    SCSpaceRotationFromRightStrategy.cpp
    SCSpaceRotationFromLeftStrategy.cpp
)

kde4_add_plugin(showcase_pageeffect_spacerotation ${showcase_pageeffect_spacerotation_PART_SRCS})

target_link_libraries(showcase_pageeffect_spacerotation showcaseprivate)

install(TARGETS showcase_pageeffect_spacerotation  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_spacerotation.desktop DESTINATION ${SERVICES_INSTALL_DIR})

