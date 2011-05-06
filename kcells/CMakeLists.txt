project(kcells)

add_subdirectory( data )
add_subdirectory( shape )
add_subdirectory( tests )
add_subdirectory( plugins )
add_subdirectory( dtd )
add_subdirectory( functions )

include_directories( ${CMAKE_SOURCE_DIR}/interfaces
                    ${CMAKE_SOURCE_DIR}/kchart
                    ${KOMAIN_INCLUDES}
                    ${KOTEXT_INCLUDES}
                    ${EIGEN2_INCLUDE_DIR} )

########### next target ###############
SET (chart_DIR_SRCS
    chart/KCChartDatabaseSelectorFactory.cpp
    chart/KCChartDatabaseSelector.cpp
    chart/KCChartDialog.cpp
    )

kde4_add_ui_files(chart_DIR_SRCS
    chart/KCChartDatabaseSelector.ui
    )

SET (commands_DIR_SRCS
    commands/KCAbstractRegionCommand.cpp
    commands/KCApplyFilterCommand.cpp
    commands/KCAutoFillCommand.cpp
    commands/KCAutoFilterCommand.cpp
    commands/KCAutoFormatCommand.cpp
    commands/KCBorderColorCommand.cpp
    commands/KCColumnStyleCommand.cpp
    commands/KCCommentCommand.cpp
    commands/ConditionCommand.cpp
    commands/KCCopyCommand.cpp
    commands/KCCSVDataCommand.cpp
    commands/DataManipulators.cpp
    commands/KCDeleteCommand.cpp
    commands/KCIndentationCommand.cpp
    commands/KCLinkCommand.cpp
    commands/KCMergeCommand.cpp
    commands/KCNamedAreaCommand.cpp
    commands/KCPageBreakCommand.cpp
    commands/KCPasteCommand.cpp
    commands/KCPrecisionCommand.cpp
    commands/RowColumnManipulators.cpp
    commands/KCRowStyleCommand.cpp
    commands/SheetCommands.cpp
    commands/SortManipulator.cpp
    commands/SpellCheckCommand.cpp
    commands/StyleCommand.cpp
    commands/ValidityCommand.cpp
    )

SET (database_DIR_SRCS
    #database/Database.cpp
    #database/DatabaseManager.cpp
    database/DatabaseSource.cpp
    database/DatabaseSourceQuery.cpp
    database/DatabaseSourceSql.cpp
    database/DatabaseSourceTable.cpp
    #database/Filter.cpp
    database/FilterPopup.cpp
    )

SET (dialogs_DIR_SRCS
    dialogs/AddNamedAreaDialog.cpp
    dialogs/AngleDialog.cpp
    dialogs/AutoFormatDialog.cpp
    dialogs/CharacterSelectDialog.cpp
    dialogs/CommentDialog.cpp
    dialogs/ConditionalDialog.cpp
    dialogs/ConsolidateDialog.cpp
    dialogs/CSVDialog.cpp
    dialogs/DatabaseDialog.cpp
    dialogs/DocumentSettingsDialog.cpp
    dialogs/FindDialog.cpp
    dialogs/FormulaDialog.cpp
    dialogs/GoalSeekDialog.cpp
    dialogs/GotoDialog.cpp
    dialogs/InsertDialog.cpp
    dialogs/LayoutDialog.cpp
    dialogs/LinkDialog.cpp
    dialogs/ListDialog.cpp
    dialogs/NamedAreaDialog.cpp
    dialogs/PasteInsertDialog.cpp
    dialogs/Resize2Dialog.cpp
    dialogs/SeriesDialog.cpp
    dialogs/ShowDialog.cpp
    dialogs/ShowColRowDialog.cpp
    dialogs/SortDialog.cpp
    dialogs/SpecialPasteDialog.cpp
    dialogs/StyleManagerDialog.cpp
    dialogs/SubtotalDialog.cpp
    dialogs/ValidityDialog.cpp
    )


kde4_add_ui_files(dialogs_DIR_SRCS
    dialogs/ConsolidateWidget.ui
    dialogs/ConsolidateDetailsWidget.ui
    dialogs/FontWidget.ui
    dialogs/GoalSeekWidget.ui
    dialogs/PositionWidget.ui
    dialogs/ProtectionWidget.ui
    dialogs/SpecialPasteWidget.ui
    dialogs/SortWidget.ui
    dialogs/SortDetailsWidget.ui
    dialogs/SubtotalWidget.ui
    dialogs/SubtotalsDetailsWidget.ui
    )

SET (functions_DIR_SRCS
    functions/helper.cpp
    )

SET (interfaces_DIR_SRCS
    interfaces/MapAdaptor.cpp
    interfaces/SheetAdaptor.cpp
    interfaces/ViewAdaptor.cpp
    )

SET (part_DIR_SRCS
    part/KCCanvas.cpp
    part/KCCellTool.cpp
    part/KCCellToolFactory.cpp
    #part/Digest.cpp
    part/KCDoc.cpp
    part/KCFactory.cpp
    part/Headers.cpp
    part/KCPrintJob.cpp
    part/KCToolRegistry.cpp
    part/KCTabBar.cpp
    part/KCView.cpp
    part/commands/DefinePrintRangeCommand.cpp
    part/commands/PageLayoutCommand.cpp
    part/dialogs/PageLayoutDialog.cpp
    part/dialogs/PreferenceDialog.cpp
    part/dialogs/SheetPropertiesDialog.cpp
    part/dialogs/SheetSelectPage.cpp
)

kde4_add_ui_files(part_DIR_SRCS
    part/dialogs/FileOptionsWidget.ui
    part/dialogs/InterfaceOptionsWidget.ui
    part/dialogs/PageLayoutSheetPage.ui
    part/dialogs/SheetPropertiesWidget.ui
    part/dialogs/SheetSelectWidget.ui
)

SET (ui_DIR_SRCS
    ui/AbstractSelectionStrategy.cpp
    ui/AutoFillStrategy.cpp
    ui/CellEditorBase.cpp
    ui/CellEditor.cpp
    ui/CellToolBase.cpp
    ui/CellToolBase_p.cpp
    ui/CellToolOptionWidget.cpp
    ui/CellView.cpp
    ui/DragAndDropStrategy.cpp
    ui/FormulaEditorHighlighter.cpp
    ui/FunctionCompletion.cpp
    ui/ExternalEditor.cpp
    ui/HyperlinkStrategy.cpp
    ui/LocationComboBox.cpp
    ui/MapViewModel.cpp
    ui/MergeStrategy.cpp
    ui/PasteStrategy.cpp
    ui/RegionSelector.cpp
    ui/RightToLeftPaintingStrategy.cpp
    ui/Selection.cpp
    ui/SelectionStrategy.cpp
    ui/SheetView.cpp
)

SET (kcellscommon_LIB_SRCS
    KCMapModel.cpp
    KCPageManager.cpp
    KCRegionModel.cpp
    tests/inspector.cpp
    ${chart_DIR_SRCS}
    ${commands_DIR_SRCS}
    ${database_DIR_SRCS}
    ${dialogs_DIR_SRCS}
    ${functions_DIR_SRCS}
    ${interfaces_DIR_SRCS}
    ${part_DIR_SRCS}
    ${ui_DIR_SRCS}
    )

set (kcellsodf_LIB_SRCS
    part/Digest.cpp
    KCApplicationSettings.cpp
    KCBinding.cpp
    KCBindingManager.cpp
    KCCalculationSettings.cpp
    KCCell.cpp
    KCCellStorage.cpp
    KCCluster.cpp
    KCCondition.cpp
    KCCurrency.cpp
    Damages.cpp
    KCDependencyManager.cpp
    KCDocBase.cpp
    KCFormat.cpp
    KCFormula.cpp
    KCGenValidationStyle.cpp
    KCHeaderFooter.cpp
    KCLocalization.cpp
    KCMap.cpp
    KCNamedAreaManager.cpp
    KCNumber.cpp
    KCPrintSettings.cpp
    KCProtectableObject.cpp
    KCRecalcManager.cpp
    KCRectStorage.cpp
    KCRegion.cpp
    RowColumnFormat.cpp
    KCRowRepeatStorage.cpp
    KCShapeApplicationData.cpp
    KCSheet.cpp
    KCSheetAccessModel.cpp
    KCSheetModel.cpp
    KCStyle.cpp
    KCStyleManager.cpp
    KCStyleStorage.cpp
    Util.cpp
    KCValidity.cpp
    KCValue.cpp
    KCValueCalc.cpp
    KCValueConverter.cpp
    KCValueFormatter.cpp
    KCValueParser.cpp

    database/Database.cpp
    database/DatabaseManager.cpp
    database/Filter.cpp

    # TODO: move the formula evaluation out of KCFormula.cpp so these files can move out of libkcellsodf
    KCFunction.cpp
    KCFunctionDescription.cpp
    KCFunctionModule.cpp
    KCFunctionModuleRegistry.cpp
    KCFunctionRepository.cpp

    # TODO: move KCHeaderFooter from KCSheetPrint to KCPrintSettings, and replace KCSheetPrint with KCPrintSettings in KCSheet to get rid of this dependency
    KCSheetPrint.cpp
    SheetPrint_p.cpp

    )


kde4_add_library(kcellsodf SHARED ${kcellsodf_LIB_SRCS})

target_link_libraries(kcellsodf komain)
target_link_libraries(kcellsodf LINK_INTERFACE_LIBRARIES komain)

set_target_properties(kcellsodf PROPERTIES
   VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kcellsodf ${INSTALL_TARGETS_DEFAULT_ARGS})


kde4_add_library(kcellscommon SHARED ${kcellscommon_LIB_SRCS})

target_link_libraries(kcellscommon
    komain
    kcellsodf
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KNOTIFYCONFIG_LIBS}
    ${KDE4_KUTILS_LIBS}
#    ${KDE4_KDE3SUPPORT_LIBS}
)

target_link_libraries(kcellscommon LINK_INTERFACE_LIBRARIES komain kcellsodf )

set_target_properties(kcellscommon PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kcellscommon  ${INSTALL_TARGETS_DEFAULT_ARGS})
########### next target ###############

SET (kcellspart_PART_SRCS part/Factory_init.cpp )

kde4_add_plugin(kcellspart ${kcellspart_PART_SRCS})



target_link_libraries(kcellspart kcellscommon )

install(TARGETS kcellspart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

SET (kcells_KDEINIT_SRCS part/Main.cpp )

kde4_add_app_icon(kcells_KDEINIT_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/data/pics/hi*-app-kcells.png")

kde4_add_kdeinit_executable( kcells ${kcells_KDEINIT_SRCS})

if (Q_WS_MAC)
set_target_properties(kcells PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
set_target_properties(kcells PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.koffice.kcells")
set_target_properties(kcells PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KCells 2")
endif (Q_WS_MAC)

target_link_libraries(kdeinit_kcells komain )

install(TARGETS kdeinit_kcells  ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kcells kdeinit_kcells )
install(TARGETS kcells  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES  kcells.rc kcells_readonly.rc kcells.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kcells)
install( FILES  kcellspart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  kcells.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  kcells.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES  kcells_plugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES  kcellsrc DESTINATION ${CONFIG_INSTALL_DIR})
