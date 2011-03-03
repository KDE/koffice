project(showcase)

include_directories( ${KOMAIN_INCLUDES} ${KOPAGEAPP_INCLUDES} ${Boost_INCLUDE_DIR})

if(KDE4_BUILD_TESTS)
    # only with this definition will the SHOWCASE_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)


add_subdirectory(tests)

### showcaseprivate ###
set( showcaseprivate_LIB_SRCS
     KPrFactory.cpp
     KPrDocument.cpp
     KPrDeclarations.cpp
     KPrView.cpp
     KPrViewAdaptor.cpp
     KPrViewModePresentation.cpp
     KPrViewModeNotes.cpp
     KPrViewModeSlidesSorter.cpp
     KPrViewModePreviewPageEffect.cpp
     KPrPresentationTool.cpp
     KPrAnimationDirector.cpp
     KPrShapeAnimations.cpp
     KPrShapeManagerAnimationStrategy.cpp
     KPrShapeManagerDisplayMasterStrategy.cpp
     KPrPageData.cpp
     KPrPage.cpp
     KPrMasterPage.cpp
     KPrNotes.cpp
     KPrSoundData.cpp
     KPrSoundCollection.cpp
     KPrEventActionData.cpp
     KPrEventActionWidget.cpp
     KPrPageApplicationData.cpp
     KPrShapeApplicationData.cpp
     KPrCustomSlideShows.cpp
     KPrPresenterViewBaseInterface.cpp
     KPrPresenterViewInterface.cpp
     KPrPresenterViewSlidesInterface.cpp
     KPrPresenterViewToolWidget.cpp
     KPrPresenterViewWidget.cpp
     KPrEndOfSlideShowPage.cpp
     KPrPlaceholderShape.cpp
     KPrPlaceholderShapeFactory.cpp
     KPrPlaceholderStrategy.cpp
     KPrPlaceholderPictureStrategy.cpp
     KPrPlaceholderTextStrategy.cpp
     KPrPresentationHighlightWidget.cpp
     KPrPresentationDrawWidget.cpp
     KPrPresentationBlackWidget.cpp
     KPrPresentationStrategy.cpp
     KPrPresentationHighlightStrategy.cpp
     KPrPresentationBlackStrategy.cpp
     KPrPresentationStrategyBase.cpp
     KPrPresentationToolAdaptor.cpp
     KPrPresentationToolEventForwarder.cpp
     KPrPresentationDrawStrategy.cpp
     KPrPageSelectStrategyBase.cpp
     KPrPageSelectStrategyFixed.cpp
     KPrPageSelectStrategyActive.cpp
     KPrDurationParser.cpp
     KPrHtmlExport.cpp
     KPrHtmlExportUiDelegate.cpp
     KPrPicturesImport.cpp
     KPrPdfPrintJob.cpp

     commands/KPrAnimationCreateCommand.cpp
     commands/KPrPageEffectSetCommand.cpp
     commands/KPrPageLayoutCommand.cpp
     commands/KPrSetCustomSlideShowsCommand.cpp

     dockers/KPrPreviewWidget.cpp
     dockers/KPrPageLayoutDockerFactory.cpp
     dockers/KPrPageLayoutDocker.cpp

     pageeffects/KPrPageEffectRunner.cpp
     pageeffects/KPrPageEffect.cpp
     pageeffects/KPrPageEffectStrategy.cpp
     pageeffects/KPrPageEffectFactory.cpp
     pageeffects/KPrPageEffectRegistry.cpp

     animations/KPrAnimationBase.cpp
     animations/KPrAnimSet.cpp
     animations/KPrAnimate.cpp
     animations/KPrAnimateColor.cpp
     animations/KPrAnimateMotion.cpp
     animations/KPrAnimateTransform.cpp
     animations/KPrAnimTransitionFilter.cpp
     animations/KPrAnimationFactory.cpp
     animations/KPrAnimationCache.cpp
     animations/KPrTextBlockPaintStrategy.cpp
     animations/KPrShapeAnimation.cpp
     animations/KPrAnimationStep.cpp
     animations/KPrAnimationSubStep.cpp
     animations/KPrAnimationLoader.cpp
     animations/KPrAnimationData.cpp

     animations/strategy/KPrAnimationValue.cpp
     animations/strategy/KPrValueParser.cpp
     animations/strategy/KPrAnimationAttribute.cpp
     animations/strategy/KPrSmilValues.cpp
     animations/strategy/KPrAttributeX.cpp
     animations/strategy/KPrAttributeY.cpp
     animations/strategy/KPrAttributeWidth.cpp
     animations/strategy/KPrAttributeHeight.cpp
     animations/strategy/KPrAttributeRotate.cpp

     pagelayout/KPrPlaceholder.cpp
     pagelayout/KPrPageLayout.cpp
     pagelayout/KPrPageLayouts.cpp
     pagelayout/KPrPageLayoutSharedSavingData.cpp
     pagelayout/KPrPlaceholders.cpp

     ui/KPrCustomSlideShowsDialog.cpp
     ui/KPrConfigureSlideShowDialog.cpp
     ui/KPrConfigurePresenterViewDialog.cpp
     ui/KPrPresentationToolWidget.cpp
     ui/KPrHtmlExportDialog.cpp

     tools/KPrPlaceholderTool.cpp
     tools/KPrPlaceholderToolFactory.cpp
     tools/KPrAnimationTool.cpp
     tools/KPrAnimationToolFactory.cpp
     tools/KPrClickActionDocker.cpp
     tools/KPrPageEffectDocker.cpp
   )

kde4_add_ui_files(showcaseprivate_LIB_SRCS
     ui/KPrCustomSlideShowsDialog.ui
     ui/KPrConfigureSlideShow.ui
     ui/KPrConfigurePresenterView.ui
     ui/KPrPresentationTool.ui
     ui/KPrHtmlExport.ui
   )

kde4_add_library(showcaseprivate SHARED ${showcaseprivate_LIB_SRCS})

target_link_libraries(showcaseprivate kopageapp kowidgets ${KDE4_PHONON_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
target_link_libraries(showcaseprivate LINK_INTERFACE_LIBRARIES kopageapp)

set_target_properties(showcaseprivate PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS showcaseprivate  ${INSTALL_TARGETS_DEFAULT_ARGS})

### showcasepart ###
set(showcasepart_PART_SRCS KPrFactoryInit.cpp )

kde4_add_plugin(showcasepart ${showcasepart_PART_SRCS})



target_link_libraries(showcasepart showcaseprivate )

install(TARGETS showcasepart DESTINATION ${PLUGIN_INSTALL_DIR})

### kdeinit showcase ###
set(showcase_KDEINIT_SRCS main.cpp )

kde4_add_app_icon(showcase_KDEINIT_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../pics/hi*-app-showcase.png")

kde4_add_kdeinit_executable( showcase ${showcase_KDEINIT_SRCS})

if (Q_WS_MAC)
   set_target_properties(showcase PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
   set_target_properties(showcase PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.koffice.showcase")
   set_target_properties(showcase PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "showcase 2")
endif (Q_WS_MAC)

target_link_libraries(kdeinit_showcase komain )

install(TARGETS kdeinit_showcase ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries(showcase kdeinit_showcase)
install(TARGETS showcase ${INSTALL_TARGETS_DEFAULT_ARGS})

### desktop files ###
install(FILES
    showcasepart.desktop
    pageeffects/kpr_pageeffect.desktop
    shapeanimations/kpr_shapeanimation.desktop
    DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS showcase.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

### GUI files ###
install( FILES showcase.rc showcase_readonly.rc DESTINATION ${DATA_INSTALL_DIR}/showcase)

install( FILES showcaserc DESTINATION ${CONFIG_INSTALL_DIR} )
