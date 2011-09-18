include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(showcase_pageeffect_barwipe_PART_SRCS Plugin.cpp
    SCBarWipeEffectFactory.cpp
    SCBarWipeFromLeftStrategy.cpp
    SCBarWipeFromRightStrategy.cpp
    SCBarWipeFromTopStrategy.cpp
    SCBarWipeFromBottomStrategy.cpp
)

kde4_add_plugin(showcase_pageeffect_barwipe ${showcase_pageeffect_barwipe_PART_SRCS})

target_link_libraries(showcase_pageeffect_barwipe showcaseprivate)

install(TARGETS showcase_pageeffect_barwipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_barwipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


