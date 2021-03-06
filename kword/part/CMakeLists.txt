project(kwordpart)

add_subdirectory( tests )

include_directories( ${KODFTEXT_INCLUDES}  ${KDE4_INCLUDE_DIR}/threadweaver/ )

########### KWordPrivate library ###############

set(kwordprivate_LIB_SRCS
    KWFactory.cpp
    KWord.cpp
    KWApplicationConfig.cpp
    KWCanvas.cpp
    KWDocument.cpp
    KWGui.cpp
    KWView.cpp
    KWPage.cpp
    KWPageManager.cpp
    KWPageStyle.cpp
    KWPageTextInfo.cpp
    KWViewMode.cpp
    KWViewModeNormal.cpp
    KWViewModePreview.cpp
    KWStatusBar.cpp
    KWOdfLoader.cpp
    KWOdfWriter.cpp
    KWOdfSharedLoadingData.cpp
    KWDLoader.cpp

    frames/KWTextDocumentLayout.cpp
    frames/KWFrameLayout.cpp
    frames/KWFrame.cpp
    frames/KWFrameSet.cpp
    frames/KWTextFrame.cpp
    frames/KWTextFrameSet.cpp
    frames/KWCopyShape.cpp
    frames/KWAnchorStrategy.cpp
    frames/KWOutlineShape.cpp
    frames/Outline.cpp
    frames/KWPageBackground.cpp

    dialogs/KWDocumentColumns.cpp
    dialogs/KWFrameConnectSelector.cpp
    dialogs/KWFrameDialog.cpp
    dialogs/KWFrameGeometry.cpp
    dialogs/KWFrameRunaroundProperties.cpp
    dialogs/KWGeneralFrameProperties.cpp
    dialogs/KWHeaderFooter.cpp
    dialogs/KWInsertPageDialog.cpp
    dialogs/KWPageSettingsDialog.cpp
    dialogs/KWPrintingDialog.cpp
    dialogs/KWShapeConfigFactory.cpp
    dialogs/KWStartupWidget.cpp
    dialogs/KWStatistics.cpp
    dialogs/KWStatisticsDialog.cpp
    dockers/KWStatisticsDocker.cpp

    commands/KWChangePageStyleCommand.cpp
    commands/KWClipFrameCommand.cpp
    commands/KWCreateOutlineCommand.cpp
    commands/KWFrameCreateCommand.cpp
    commands/KWFrameDeleteCommand.cpp
    commands/KWFrameRemoveSilentCommand.cpp
    commands/KWNewPageStyleCommand.cpp
    commands/KWPageInsertCommand.cpp
    commands/KWPageRemoveCommand.cpp
    commands/KWPageStylePropertiesCommand.cpp
    commands/KWRemoveFrameClipCommand.cpp
    commands/KWSetFrameSetCommand.cpp
    commands/KWSetFSNameCommand.cpp
)
if( Soprano_FOUND )
    set(kwordprivate_LIB_SRCS ${kwordprivate_LIB_SRCS}
        dockers/KWRdfDocker.cpp
        dockers/KWRdfDockerFactory.cpp
        dockers/KWRdfDockerTree.cpp
    )
endif( Soprano_FOUND )

kde4_add_ui_files(kwordprivate_LIB_SRCS
    dialogs/KWDocumentColumns.ui
    dialogs/KWFrameConnectSelector.ui
    dialogs/KWFrameGeometry.ui
    dialogs/KWFrameRunaroundProperties.ui
    dialogs/KWGeneralFrameProperties.ui
    dialogs/KWHeaderFooter.ui
    dialogs/KWInsertPageDialog.ui
    dialogs/KWStartupWidget.ui
    dialogs/KWStatistics.ui
    dockers/KWStatisticsDocker.ui
)
if( Soprano_FOUND )
    kde4_add_ui_files(kwordprivate_LIB_SRCS
        dockers/KWRdfDocker.ui
    )
endif( Soprano_FOUND )


kde4_add_library(kwordprivate SHARED ${kwordprivate_LIB_SRCS})

target_link_libraries(kwordprivate komain)
if( Soprano_FOUND )
    target_link_libraries(kwordprivate ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )
target_link_libraries(kwordprivate LINK_INTERFACE_LIBRARIES komain)

set_target_properties(kwordprivate
    PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kwordprivate  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### KWordPart Library ###############

set(kwordpart_PART_SRCS KWFactoryInit.cpp )

kde4_add_plugin(kwordpart ${kwordpart_PART_SRCS})

target_link_libraries(kwordpart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kwordprivate )

install(TARGETS kwordpart  DESTINATION ${PLUGIN_INSTALL_DIR})

########### KWord exe ###############

set(kword_KDEINIT_SRCS main.cpp )

kde4_add_app_icon(kword_KDEINIT_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kword.png")

kde4_add_kdeinit_executable( kword ${kword_KDEINIT_SRCS})

if (Q_WS_MAC)
   set_target_properties(kword PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
   set_target_properties(kword PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.koffice.kword")
   set_target_properties(kword PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KWord 2")
endif (Q_WS_MAC)

target_link_libraries(kdeinit_kword  ${KDE4_KDEUI_LIBS} komain )

install(TARGETS kdeinit_kword  ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kword kdeinit_kword )
install(TARGETS kword  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kword.rc kword_readonly.rc  DESTINATION ${DATA_INSTALL_DIR}/kword)
install( FILES kwordpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS kword.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kwordrc DESTINATION ${CONFIG_INSTALL_DIR} )

