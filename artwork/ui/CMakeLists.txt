include_directories(
        ${KDE4_INCLUDE_DIR}
        ${QT_INCLUDES}
        ${KOMAIN_INCLUDES}
        ${CMAKE_SOURCE_DIR}/artwork/
        ${CMAKE_SOURCE_DIR}/artwork/ui
        ${CMAKE_SOURCE_DIR}/artwork/ui/dialogs
        ${CMAKE_SOURCE_DIR}/artwork/ui/dockers
        ${CMAKE_SOURCE_DIR}/artwork/ui/widgets
)

set(artworkui_SRCS
    ArtworkFactory.cpp
    ArtworkPart.cpp
    ArtworkView.cpp
    ArtworkPrintJob.cpp
    ArtworkZoomController.cpp
    ArtworkDocumentMergeCommand.cpp

    dialogs/ArtworkConfigureDialog.cpp

    dockers/ArtworkLayerDocker.cpp
    dockers/ArtworkLayerModel.cpp
    dockers/ArtworkLayerSortingModel.cpp

    widgets/ArtworkCanvas.cpp
    widgets/ArtworkSmallStylePreview.cpp
    widgets/ArtworkPatternOptionsWidget.cpp
)

kde4_add_ui_files(artworkui_SRCS
    widgets/ArtworkPatternOptionsWidget.ui
)


kde4_add_library( artworkui SHARED ${artworkui_SRCS} )

target_link_libraries(artworkui artworkcommon kowidgets)
target_link_libraries(artworkui LINK_INTERFACE_LIBRARIES artworkcommon)

set_target_properties(artworkui PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS artworkui  ${INSTALL_TARGETS_DEFAULT_ARGS})

