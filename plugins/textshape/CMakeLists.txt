project( textPlugin)

add_subdirectory( pics )
add_subdirectory( tests )

include_directories(
    ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets/
)

########### Flake Plugin library ###############

SET ( textshape_SRCS
    TextPlugin.cpp
    TextShape.cpp
    TextShapeFactory.cpp
    TextTool.cpp
    TextEditingPluginContainer.cpp
    TextToolFactory.cpp
    ToCGenerator.cpp
    Layout.cpp
    ListItemsHelper.cpp

    ChangeTracker.cpp
    ChangeTrackingTool.cpp
    ChangeTrackingToolFactory.cpp
    TextChanges.cpp
    TextChange.cpp

    TableLayout.cpp
    TableLayoutData.cpp

    dialogs/ChangeConfigureDialog.cpp
    dialogs/CharacterGeneral.cpp
    dialogs/CharacterHighlighting.cpp
    dialogs/CreateBookmark.cpp
    dialogs/FontDecorations.cpp
    dialogs/FontDia.cpp
    dialogs/FontLayoutTab.cpp
    dialogs/FontTab.cpp
    dialogs/FormattingPreview.cpp
    dialogs/InsertCharacter.cpp
    dialogs/JumpOverview.cpp
    dialogs/LanguageTab.cpp
    dialogs/ListsSpinBox.cpp
    dialogs/NewStyleWidget.cpp
    dialogs/ParagraphBulletsNumbers.cpp
    dialogs/ParagraphDecorations.cpp
    dialogs/ParagraphGeneral.cpp
    dialogs/ParagraphIndentSpacing.cpp
    dialogs/ParagraphLayout.cpp
    dialogs/ParagraphSettingsDialog.cpp
    dialogs/SimpleStyleWidget.cpp
    dialogs/StyleManager.cpp
    dialogs/StyleManagerDialog.cpp
    dialogs/StyleManagerWelcome.cpp
    dialogs/StylesModel.cpp
    dialogs/StylesWidget.cpp
    dialogs/TableDialog.cpp
    dialogs/TrackedChangeManager.cpp
    dialogs/TrackedChangeModel.cpp

    commands/ChangeListCommand.cpp
    commands/ChangeListLevelCommand.cpp
    commands/ListItemNumberingCommand.cpp
    commands/TextCommandBase.cpp
    commands/TextPasteCommand.cpp
    commands/TextCutCommand.cpp
    commands/ShowChangesCommand.cpp
    commands/ChangeTrackedDeleteCommand.cpp
    commands/DeleteCommand.cpp
    commands/AcceptChangeCommand.cpp
    commands/RejectChangeCommand.cpp
)

kde4_add_ui_files(textshape_SRCS
    dialogs/CharacterGeneral.ui
    dialogs/CharacterHighlighting.ui
    dialogs/CreateBookmark.ui
    dialogs/JumpOverview.ui
    dialogs/NewStyleWidget.ui
    dialogs/ParagraphBulletsNumbers.ui
    dialogs/ParagraphDecorations.ui
    dialogs/ParagraphGeneral.ui
    dialogs/ParagraphIndentSpacing.ui
    dialogs/ParagraphLayout.ui
    dialogs/SimpleStyleWidget.ui
    dialogs/StyleManager.ui
    dialogs/StyleManagerWelcome.ui
    dialogs/StylesWidget.ui
    dialogs/TableDialog.ui

    dialogs/FontDecorations.ui
    dialogs/FontLayoutTab.ui
    dialogs/LanguageTab.ui
    dialogs/ChangeConfigureDialog.ui
    dialogs/TrackedChangeManager.ui
)

kde4_add_plugin(textshape ${textshape_SRCS})

target_link_libraries(textshape komain ${EXTRA_LIBS})
if( Soprano_FOUND )
    target_link_libraries(textshape ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )


install(TARGETS textshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES textshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
