include_directories(${KOTEXT_INCLUDES})

add_subdirectory( tests )

########### next target ###############
set(kotext_LIB_SRCS
    KoTextShapeData.cpp
    KoText.cpp
    KoTextBlockData.cpp
    KoTextBlockBorderData.cpp
    KoTextBlockPaintStrategyBase.cpp
    KoTextDocumentLayout.cpp
    KoTextOdfSaveHelper.cpp
    KoTextPaste.cpp
    KoTextDocument.cpp
    KoTextEditor.cpp
    KoList.cpp

    KoTextEditingRegistry.cpp
    KoTextEditingFactory.cpp
    KoTextEditingPlugin.cpp

    KInlineTextObjectManager.cpp
    KInlineObjectFactoryBase.cpp
    KInlineObjectRegistry.cpp
    InsertInlineObjectActionBase.cpp
    InsertVariableAction.cpp
    InsertNamedVariableAction.cpp
    InsertTextReferenceAction.cpp
    InsertTextLocator.cpp
    KInlineObject.cpp
    KoVariable.cpp
    KoVariableManager.cpp
    KNamedVariable.cpp
    KoTextLocator.cpp
    KoTextReference.cpp
    KoTextAnchor.cpp
    KoTextShapeContainerModel.cpp
    KoTextShapeSavingContext.cpp
    KoBookmark.cpp
    KoBookmarkManager.cpp
    KInlineNote.cpp
    FindDirection_p.cpp
    KFindStrategy.cpp
    KReplaceStrategy.cpp
    KoFind_p.cpp
    KoFind.cpp
    KoTextDebug.cpp
    KoTextPage.cpp
    KPageProvider.cpp
    KoTableColumnAndRowStyleManager.cpp
    KoTextInlineRdf.cpp
    KoTextMeta.cpp
    KDocumentRdfBase.cpp

    styles/Styles_p.cpp
    styles/KCharacterStyle.cpp
    styles/KParagraphStyle.cpp
    styles/KStyleManager.cpp
    styles/KListStyle.cpp
    styles/KListLevelProperties.cpp
    styles/ChangeFollower.cpp
    styles/KTableBorderStyle.cpp
    styles/KoTableStyle.cpp
    styles/KoTableColumnStyle.cpp
    styles/KoTableRowStyle.cpp
    styles/KTableCellStyle.cpp
    styles/KoTableFormat.cpp
    styles/KoTableFormat_p.cpp
    styles/KSectionStyle.cpp
    opendocument/KoTextSharedLoadingData.cpp
    opendocument/KoTextSharedSavingData.cpp
    opendocument/KoTextLoader.cpp
    opendocument/KoTextWriter.cpp

    changetracker/KChangeTracker.cpp
    changetracker/KChangeTrackerElement.cpp
    changetracker/KDeleteChangeMarker.cpp
    changetracker/KFormatChangeInformation.cpp
    changetracker/KDeletedRowColumnDataStore.cpp
    changetracker/KDeletedRowData.cpp
    changetracker/KDeletedColumnData.cpp
    changetracker/KDeletedCellData.cpp
    KoTextDrag.cpp

    commands/DeleteTableRowCommand.cpp
    commands/DeleteTableColumnCommand.cpp
    commands/InsertTableRowCommand.cpp
    commands/InsertTableColumnCommand.cpp
)

if( Soprano_FOUND )
    set(kotext_LIB_SRCS ${kotext_LIB_SRCS}
        KoTextRdfCore.cpp
    )
endif( Soprano_FOUND )

kde4_add_library(kotext SHARED ${kotext_LIB_SRCS})

target_link_libraries(kotext flake)
if( Soprano_FOUND )
    target_link_libraries(kotext ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )
target_link_libraries(kotext LINK_INTERFACE_LIBRARIES flake)

set_target_properties(kotext PROPERTIES
   VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kotext  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES texteditingplugin.desktop inlinetextobject.desktop textvariableplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(
    FILES
    kotext_export.h
    KInlineObject.h
    KoTextEditor.h
    KoTextEditingFactory.h
    KoTextEditingPlugin.h
    KoTextEditingRegistry.h
    KInlineObjectRegistry.h
    KInlineObjectFactoryBase.h
    KoBookmark.h
    KoBookmarkManager.h
    KInlineTextObjectManager.h
    KoTextAnchor.h
    KoTextBlockBorderData.h
    KoTextBlockData.h
    KoTextDocument.h
    KoText.h
    KoList.h
    KoTextLocator.h
    KoTextPage.h
    KoTextPaste.h
    KoTextShapeData.h
    KoVariable.h
    KoVariableManager.h
    KoTextRdfCore.h
    KoTextInlineRdf.h
    KoTextMeta.h
    KoTextDocumentLayout.h
    styles/KCharacterStyle.h
    styles/KListLevelProperties.h
    styles/KListStyle.h
    styles/KParagraphStyle.h
    styles/KoTableColumnStyle.h
    styles/KoTableRowStyle.h
    styles/KTableCellStyle.h
    styles/KoTableFormat.h
    styles/KSectionStyle.h
    styles/KStyleManager.h
    changetracker/KChangeTracker.h
    changetracker/KChangeTrackerElement.h
    changetracker/KDeleteChangeMarker.h
    changetracker/KDeletedRowData.cpp
    changetracker/KDeletedCellData.cpp
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
