add_subdirectory( tests )
add_subdirectory( pics )

include_directories(${KODFTEXT_INCLUDES} ${KODF_INCLUDES} ${PIGMENT_INCLUDES})

set(kowidgets_LIB_SRCS
    KoAspectButton.cpp
    KoCsvImportDialog.cpp
    KoPageLayoutDialog.cpp
    KoPageLayoutWidget.cpp
    KoPagePreviewWidget.cpp
    KoPositionSelector.cpp
    KoSliderCombo.cpp
    KoUnitDoubleSpinBox.cpp
    KoZoomAction.cpp
    KoZoomController.cpp
    KoZoomInput.cpp
    KoZoomHandler.cpp
    KoZoomMode.cpp
    KoDpi.cpp

    KoColorPatch.cpp
    KoColorPopupAction.cpp
    KoColorSetWidget.cpp
    KoColorSlider.cpp
    KoTriangleColorSelector.cpp
    KoEditColorSetDialog.cpp
    KoShadowConfigWidget.cpp
    KoIconToolTip.cpp
    KoResourceItemChooser.cpp
    KoResourceSelector.cpp
    KoResourceModel.cpp
    KoResourceItemDelegate.cpp
    KoResourceItemView.cpp
    KoItemToolTip.cpp
    KoCheckerBoardPainter.cpp
    KoResourceServerAdapter.cpp
    KoResourceServerProvider.cpp
    KoDockWidgetTitleBar.cpp
    KoDockWidgetTitleBarButton.cpp
)

kde4_add_ui_files( kowidgets_LIB_SRCS
    KoCsvImportDialog.ui
    KoPageLayoutWidget.ui
    KoEditColorSet.ui
    KoShadowConfigWidget.ui
)

kde4_add_library(kowidgets SHARED ${kowidgets_LIB_SRCS})

option(GHNS "support Get Hot New Stuff" ON)
if(GHNS)
    target_link_libraries(kowidgets pigmentcms kflake ${KDE4_KNEWSTUFF3_LIBS})
else(GHNS)
    target_link_libraries(kowidgets pigmentcms kflake)
endif (GHNS)

target_link_libraries(kowidgets LINK_INTERFACE_LIBRARIES kflake pigmentcms)

set_target_properties(kowidgets PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS kowidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    KoAspectButton.h
    KoCheckerBoardPainter.h
    KoColorPatch.h
    KoColorPopupAction.h
    KoColorSetWidget.h
    KoColorSlider.h
    KoCsvImportDialog.h
    KoDockWidgetTitleBarButton.h
    KoDockWidgetTitleBar.h
    KoDpi.h
    KoEditColorSetDialog.h
    KoPageLayoutDialog.h
    KoPageLayoutWidget.h
    KoPagePreviewWidget.h
    KoPositionSelector.h
    KoResourceItemChooser.h
    KoResourceSelector.h
    KoResourceServerAdapter.h
    KoResourceServer.h
    KoResourceServerObserver.h
    KoResourceServerProvider.h
    KoShadowConfigWidget.h
    KoSliderCombo.h
    KoTriangleColorSelector.h
    KoUnitDoubleSpinBox.h
    kowidgets_export.h
    KoZoomAction.h
    KoZoomController.h
    KoZoomHandler.h
    KoZoomMode.h

    DESTINATION
    ${INCLUDE_INSTALL_DIR}/private
COMPONENT Devel)
