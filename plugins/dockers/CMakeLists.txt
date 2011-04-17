project(kofficedockers)

include_directories(${KDE4_INCLUDE_DIR}
     ${FLAKE_INCLUDES}
)

set(kofficedockers_SRCS
    shapeproperties/ShapePropertiesDocker.cpp
    shapeproperties/ShapePropertiesDockerFactory.cpp

    shapecollection/CollectionShapeFactory.cpp
    shapecollection/ShapeCollectionDocker.cpp
    shapecollection/CollectionItemModel.cpp
    shapecollection/OdfCollectionLoader.cpp

    shadowdocker/ShadowDocker.cpp
    shadowdocker/ShadowDockerFactory.cpp

    shapeselector/Canvas.cpp
    shapeselector/DragCanvasStrategy.cpp
    shapeselector/FolderBorder.cpp
    shapeselector/FolderShape.cpp
    shapeselector/FolderShapeModel.cpp
    shapeselector/GroupShape.cpp
    shapeselector/IconShape.cpp
    shapeselector/ItemStore.cpp
    shapeselector/MoveFolderStrategy.cpp
    shapeselector/RightClickStrategy.cpp
    shapeselector/SelectStrategy.cpp
    shapeselector/ShapeSelector.cpp
    shapeselector/ShapeSelectorFactory.cpp
    shapeselector/TemplateShape.cpp
    shapeselector/ZoomHandler.cpp
    shapeselector/ClipboardProxyShape.cpp
    shapeselector/ResizeFolderStrategy.cpp

    Plugin.cpp
)

kde4_add_plugin(kofficedockers ${kofficedockers_SRCS})

target_link_libraries(kofficedockers komain)

install(TARGETS kofficedockers DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficedockers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
