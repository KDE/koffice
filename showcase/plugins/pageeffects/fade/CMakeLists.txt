include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(showcase_pageeffect_fade_PART_SRCS Plugin.cpp
    SCFadeEffectFactory.cpp
    SCFadeCrossStrategy.cpp
    SCFadeOverColorStrategy.cpp
)

kde4_add_plugin(showcase_pageeffect_fade ${showcase_pageeffect_fade_PART_SRCS})

target_link_libraries(showcase_pageeffect_fade showcaseprivate)

install(TARGETS showcase_pageeffect_fade  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_fade.desktop DESTINATION ${SERVICES_INSTALL_DIR})


