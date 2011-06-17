include_directories(${KOTEXT_INCLUDES})

add_subdirectory( tests )

########### next target ###############
set(kotext_LIB_SRCS
    KTextShapeData.cpp
    KoText.cpp
    KTextBlockData.cpp
    KTextBlockBorderData.cpp
    KTextBlockPaintStrategyBase.cpp
    KTextDocumentLayout.cpp
    KTextOdfSaveHelper.cpp
    KTextPaste.cpp
    KTextDocument.cpp
    KoTextEditor.cpp
    KoList.cpp

    KTextEditingRegistry.cpp
    KTextEditingFactory.cpp
    KTextEditingPlugin.cpp

    KInlineTextObjectManager.cpp
    KInlineObjectFactoryBase.cpp
    KInlineObjectRegistry.cpp
    InsertInlineObjectActionBase.cpp
    InsertVariableAction.cpp
    InsertNamedVariableAction.cpp
    InsertTextReferenceAction.cpp
    InsertTextLocator.cpp
    KInlineObject.cpp
    KVariable.cpp
    KVariableManager.cpp
    KNamedVariable.cpp
    KTextLocator.cpp
    KTextReference.cpp
    KTextAnchor.cpp
    KTextShapeContainerModel.cpp
    KTextShapeSavingContext.cpp
    KoBookmark.cpp
    KoBookmarkManager.cpp
    KInlineNote.cpp
    FindDirection_p.cpp
    KFindStrategy.cpp
    KReplaceStrategy.cpp
    KoFind_p.cpp
    KoFind.cpp
    KTextDebug.cpp
    KTextPage.cpp
    KPageProvider.cpp
    KTableColumnAndRowStyleManager.cpp
    KTextInlineRdf.cpp
    KTextMeta.cpp
    KDocumentRdfBase.cpp

    styles/Styles_p.cpp
    styles/KCharacterStyle.cpp
    styles/KParagraphStyle.cpp
    styles/KStyleManager.cpp
    styles/KListStyle.cpp
    styles/KListLevelProperties.cpp
    styles/ChangeFollower.cpp
    styles/KTableBorderStyle.cpp
    styles/KTableStyle.cpp
    styles/KTableColumnStyle.cpp
    styles/KTableRowStyle.cpp
    styles/KTableCellStyle.cpp
    styles/KTableFormat.cpp
    styles/KTableFormat_p.cpp
    styles/KSectionStyle.cpp
    opendocument/KTextSharedLoadingData.cpp
    opendocument/KTextSharedSavingData.cpp
    opendocument/KTextLoader.cpp
    opendocument/KTextWriter.cpp

    changetracker/KChangeTracker.cpp
    changetracker/KChangeTrackerElement.cpp
    changetracker/KDeleteChangeMarker.cpp
    changetracker/KFormatChangeInformation.cpp
    changetracker/KDeletedRowColumnDataStore.cpp
    changetracker/KDeletedRowData.cpp
    changetracker/KDeletedColumnData.cpp
    changetracker/KDeletedCellData.cpp
    KTextDrag.cpp

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
    KTextEditingFactory.h
    KTextEditingPlugin.h
    KTextEditingRegistry.h
    KInlineObjectRegistry.h
    KInlineObjectFactoryBase.h
    KoBookmark.h
    KoBookmarkManager.h
    KInlineTextObjectManager.h
    KTextAnchor.h
    KTextBlockBorderData.h
    KTextBlockData.h
    KTextDocument.h
    KoText.h
    KoList.h
    KTextLocator.h
    KTextPage.h
    KTextPaste.h
    KTextShapeData.h
    KVariable.h
    KVariableManager.h
    KoTextRdfCore.h
    KTextInlineRdf.h
    KTextMeta.h
    KTextDocumentLayout.h
    styles/KCharacterStyle.h
    styles/KListLevelProperties.h
    styles/KListStyle.h
    styles/KParagraphStyle.h
    styles/KTableColumnStyle.h
    styles/KTableRowStyle.h
    styles/KTableCellStyle.h
    styles/KTableFormat.h
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
