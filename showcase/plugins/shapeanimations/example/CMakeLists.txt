include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  )

set(showcase_shapeanimation_example_PART_SRCS Plugin.cpp )

kde4_add_plugin(showcase_shapeanimation_example ${showcase_shapeanimation_example_PART_SRCS})

target_link_libraries(
    showcase_shapeanimation_example
    showcaseprivate
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    )

install(TARGETS showcase_shapeanimation_example  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_shapeanimation_example.desktop DESTINATION ${SERVICES_INSTALL_DIR})


