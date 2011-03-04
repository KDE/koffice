include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  )

set(showcase_pageeffect_example_PART_SRCS Plugin.cpp )

kde4_add_plugin(showcase_pageeffect_example ${showcase_pageeffect_example_PART_SRCS})

target_link_libraries(showcase_pageeffect_example showcaseprivate flake )

install(TARGETS showcase_pageeffect_example  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_example.desktop DESTINATION ${SERVICES_INSTALL_DIR})


