include_directories(
    ${QT_INCLUDES} 
    ${KDE4_INCLUDE_DIR}
    ${FLAKE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/artwork 
    ${CMAKE_SOURCE_DIR}/artwork/ui
    ${CMAKE_SOURCE_DIR}/artwork/ui/widgets 
    ${CMAKE_CURRENT_SOURCE_DIR}/filterEffectTool
)

########### next target ###############

set(artwork_tools_PART_SRCS
   ArtworkToolsPlugin.cpp
   ArtworkCurveFit.cpp
   ArtworkPencilTool.cpp
   ArtworkPencilToolFactory.cpp
   CalligraphyTool/ArtworkCalligraphyTool.cpp
   CalligraphyTool/ArtworkCalligraphyOptionWidget.cpp
   CalligraphyTool/ArtworkCalligraphyToolFactory.cpp
   CalligraphyTool/ArtworkCalligraphicShape.cpp
   CalligraphyTool/ArtworkCalligraphicShapeFactory.cpp
   CalligraphyTool/ArtworkSimplifyPath.cpp
   filterEffectTool/ArtworkFilterEffectsTool.cpp
   filterEffectTool/ArtworkFilterEffectsToolFactory.cpp
   filterEffectTool/FilterEffectEditWidget.cpp
   filterEffectTool/FilterEffectScene.cpp
   filterEffectTool/FilterEffectSceneItems.cpp
   filterEffectTool/FilterInputChangeCommand.cpp
   filterEffectTool/FilterAddCommand.cpp
   filterEffectTool/FilterRemoveCommand.cpp
   filterEffectTool/FilterStackSetCommand.cpp
   filterEffectTool/FilterRegionChangeCommand.cpp
   filterEffectTool/FilterEffectResource.cpp
   filterEffectTool/FilterResourceServerProvider.cpp
   filterEffectTool/FilterRegionEditStrategy.cpp
)

kde4_add_ui_files(artwork_tools_PART_SRCS
    filterEffectTool/FilterEffectEditWidget.ui
)

kde4_add_plugin(artworktools ${artwork_tools_PART_SRCS})

target_link_libraries(artworktools artworkcommon artworkui kowidgets)

install(TARGETS artworktools DESTINATION ${PLUGIN_INSTALL_DIR})

kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons  )

########### install files ###############

install( FILES artworktools.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
