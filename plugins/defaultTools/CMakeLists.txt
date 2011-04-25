project( defaulttools )

include_directories( ${FLAKE_INCLUDES} ${QT_INCLUDES} ${KOMAIN_INCLUDES} )

add_subdirectory(pics)


########### Flake Plugin library ###############

SET ( defaulttools_SRCS
    Plugin.cpp
    defaulttool/DefaultTool.cpp
    defaulttool/DefaultToolFactory.cpp
    defaulttool/DefaultToolWidget.cpp
    defaulttool/DefaultToolArrangeWidget.cpp
    defaulttool/DefaultToolTransformWidget.cpp
    defaulttool/ShapeMoveStrategy.cpp
    defaulttool/ShapeResizeStrategy.cpp
    defaulttool/ShapeRotateStrategy.cpp
    defaulttool/ShapeShearStrategy.cpp
    defaulttool/SelectionDecorator.cpp
    defaulttool/SelectionTransformCommand.cpp
    defaulttool/ConnectionChangeStrategy.cpp
    defaulttool/ConnectionChangeCommand.cpp

    guidestool/GuidesTool.cpp
    guidestool/GuidesToolFactory.cpp
    guidestool/GuidesToolOptionWidget.cpp
    guidestool/InsertGuidesToolOptionWidget.cpp
)

kde4_add_ui_files(defaulttools_SRCS
    defaulttool/DefaultToolWidget.ui
    defaulttool/DefaultToolArrangeWidget.ui
    defaulttool/DefaultToolTransformWidget.ui
    guidestool/GuidesToolOptionWidget.ui
    guidestool/InsertGuidesToolOptionWidget.ui
)

kde4_add_plugin(defaulttools ${defaulttools_SRCS})
target_link_libraries(defaulttools flake kowidgets)

########### install files ###############

install(TARGETS defaulttools DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES defaulttools.desktop DESTINATION ${SERVICES_INSTALL_DIR})
