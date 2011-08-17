include_directories(
        ${KDE4_INCLUDE_DIR}
        ${QT_INCLUDES}
        ${KOMAIN_INCLUDES}
        ${CMAKE_SOURCE_DIR}/artwork/
        ${CMAKE_SOURCE_DIR}/artwork/ui
)

set(artworkui_SRCS
    ArtworkFactory.cpp
    ArtworkPart.cpp
    ArtworkView.cpp
    ArtworkPrintJob.cpp
    ArtworkZoomController.cpp
    ArtworkDocumentMergeCommand.cpp

    ArtworkConfigureDialog.cpp

    ArtworkLayerDocker.cpp
    ArtworkLayerModel.cpp
    ArtworkLayerSortingModel.cpp

    ArtworkCanvas.cpp
    ArtworkSmallStylePreview.cpp
    ArtworkPatternOptionsWidget.cpp
)

kde4_add_ui_files(artworkui_SRCS
    ArtworkPatternOptionsWidget.ui
)

kde4_add_library( artworkui SHARED ${artworkui_SRCS} )

target_link_libraries(artworkui artworkcommon kowidgets)
target_link_libraries(artworkui LINK_INTERFACE_LIBRARIES artworkcommon)

set_target_properties(artworkui PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS artworkui  ${INSTALL_TARGETS_DEFAULT_ARGS})

