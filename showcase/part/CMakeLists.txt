project(showcase)

include_directories( ${KOMAIN_INCLUDES} ${KOPAGEAPP_INCLUDES} ${Boost_INCLUDE_DIR})

if(KDE4_BUILD_TESTS)
    # only with this definition will the SHOWCASE_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)


add_subdirectory(tests)

### showcaseprivate ###
set( showcaseprivate_LIB_SRCS
     SCFactory.cpp
     SCDocument.cpp
     SCDeclarations.cpp
     SCView.cpp
     SCViewAdaptor.cpp
     SCViewModePresentation.cpp
     SCViewModeNotes.cpp
     SCViewModeSlidesSorter.cpp
     SCViewModePreviewPageEffect.cpp
     SCPresentationTool.cpp
     SCAnimationDirector.cpp
     SCShapeAnimations.cpp
     SCShapeManagerAnimationStrategy.cpp
     SCShapeManagerDisplayMasterStrategy.cpp
     SCPageData.cpp
     SCPage.cpp
     SCMasterPage.cpp
     SCNotes.cpp
     SCSoundData.cpp
     SCSoundCollection.cpp
     SCEventActionData.cpp
     SCEventActionWidget.cpp
     SCPageApplicationData.cpp
     SCShapeApplicationData.cpp
     SCCustomSlideShows.cpp
     SCPresenterViewBaseInterface.cpp
     SCPresenterViewInterface.cpp
     SCPresenterViewSlidesInterface.cpp
     SCPresenterViewToolWidget.cpp
     SCPresenterViewWidget.cpp
     SCEndOfSlideShowPage.cpp
     SCPlaceholderShape.cpp
     SCPlaceholderShapeFactory.cpp
     SCPlaceholderStrategy.cpp
     SCPlaceholderPictureStrategy.cpp
     SCPlaceholderTextStrategy.cpp
     SCPresentationHighlightWidget.cpp
     SCPresentationDrawWidget.cpp
     SCPresentationBlackWidget.cpp
     SCPresentationStrategy.cpp
     SCPresentationHighlightStrategy.cpp
     SCPresentationBlackStrategy.cpp
     SCPresentationStrategyBase.cpp
     SCPresentationToolAdaptor.cpp
     SCPresentationToolEventForwarder.cpp
     SCPresentationDrawStrategy.cpp
     SCPageSelectStrategyBase.cpp
     SCPageSelectStrategyFixed.cpp
     SCPageSelectStrategyActive.cpp
     SCDurationParser.cpp
     SCHtmlExport.cpp
     SCHtmlExportUiDelegate.cpp
     SCPicturesImport.cpp
     SCPdfPrintJob.cpp

     commands/SCAnimationCreateCommand.cpp
     commands/SCPageEffectSetCommand.cpp
     commands/SCPageLayoutCommand.cpp
     commands/SCSetCustomSlideShowsCommand.cpp

     dockers/SCPreviewWidget.cpp
     dockers/SCPageLayoutDockerFactory.cpp
     dockers/SCPageLayoutDocker.cpp

     pageeffects/SCPageEffectRunner.cpp
     pageeffects/SCPageEffect.cpp
     pageeffects/SCPageEffectStrategy.cpp
     pageeffects/SCPageEffectFactory.cpp
     pageeffects/SCPageEffectRegistry.cpp

     animations/SCAnimationBase.cpp
     animations/SCAnimSet.cpp
     animations/SCAnimate.cpp
     animations/SCAnimateColor.cpp
     animations/SCAnimateMotion.cpp
     animations/SCAnimateTransform.cpp
     animations/SCAnimTransitionFilter.cpp
     animations/SCAnimationFactory.cpp
     animations/SCAnimationCache.cpp
     animations/SCTextBlockPaintStrategy.cpp
     animations/SCShapeAnimation.cpp
     animations/SCAnimationStep.cpp
     animations/SCAnimationSubStep.cpp
     animations/SCAnimationLoader.cpp
     animations/SCAnimationData.cpp

     animations/strategy/SCAnimationValue.cpp
     animations/strategy/SCValueParser.cpp
     animations/strategy/SCAnimationAttribute.cpp
     animations/strategy/SCSmilValues.cpp
     animations/strategy/SCAttributeX.cpp
     animations/strategy/SCAttributeY.cpp
     animations/strategy/SCAttributeWidth.cpp
     animations/strategy/SCAttributeHeight.cpp
     animations/strategy/SCAttributeRotate.cpp

     pagelayout/SCPlaceholder.cpp
     pagelayout/SCPageLayout.cpp
     pagelayout/SCPageLayouts.cpp
     pagelayout/SCPageLayoutSharedSavingData.cpp
     pagelayout/SCPlaceholders.cpp

     ui/SCCustomSlideShowsDialog.cpp
     ui/SCConfigureSlideShowDialog.cpp
     ui/SCConfigurePresenterViewDialog.cpp
     ui/SCPresentationToolWidget.cpp
     ui/SCHtmlExportDialog.cpp

     tools/SCPlaceholderTool.cpp
     tools/SCPlaceholderToolFactory.cpp
     tools/SCAnimationTool.cpp
     tools/SCAnimationToolFactory.cpp
     tools/SCClickActionDocker.cpp
     tools/SCPageEffectDocker.cpp
   )

kde4_add_ui_files(showcaseprivate_LIB_SRCS
     ui/SCCustomSlideShowsDialog.ui
     ui/SCConfigureSlideShow.ui
     ui/SCConfigurePresenterView.ui
     ui/SCPresentationTool.ui
     ui/SCHtmlExport.ui
   )

kde4_add_library(showcaseprivate SHARED ${showcaseprivate_LIB_SRCS})

target_link_libraries(showcaseprivate kopageapp kowidgets ${KDE4_PHONON_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
target_link_libraries(showcaseprivate LINK_INTERFACE_LIBRARIES kopageapp)

set_target_properties(showcaseprivate PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS showcaseprivate  ${INSTALL_TARGETS_DEFAULT_ARGS})

### showcasepart ###
set(showcasepart_PART_SRCS SCFactoryInit.cpp )

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
    DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS showcase.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

install(FILES
    pageeffects/showcase_pageeffect.desktop
    shapeanimations/showcase_shapeanimation.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})


### GUI files ###
install( FILES showcase.rc showcase_readonly.rc DESTINATION ${DATA_INSTALL_DIR}/showcase)

install( FILES showcaserc DESTINATION ${CONFIG_INSTALL_DIR} )
