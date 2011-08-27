add_subdirectory( tests )

include_directories( ${KODF_INCLUDES} )

########### libkodf ###############

if( QCA2_FOUND )
    include_directories( ${QCA2_INCLUDE_DIR} )
    add_definitions( -DQCA2 )
endif( QCA2_FOUND )

set(kodf_LIB_SRCS
    KOdfBorders.cpp
    DirectoryStore.cpp
    KOdfEmbeddedDocumentSaver.cpp
    EncryptedStore.cpp
    KOdfFontData.cpp
    KOdfGenericChange.cpp
    KOdfGenericChanges.cpp
    KOdfGenericStyle.cpp
    KOdfGenericStyles.cpp
    KOdfSettings.cpp
    KOdf.cpp
    KOdfDocumentBase.cpp
    KOdfGraphicStyles.cpp
    KOdfLoadingContext.cpp
    KOdfNumberStyles.cpp
    KOdfPasteBase.cpp
    KOdfStoreReader.cpp
    KOdfStylesReader.cpp
    KOdfWriteStore.cpp
    KOdfPageFormat.cpp
    KOdfPageLayoutData.cpp
    KProperties.cpp
    KOdfStore.cpp
    KOdfStorageDevice.cpp
    KOdfStyleStack.cpp
    TarStore.cpp
    KUnit.cpp
    KOdfXmlNS.cpp
    KXmlReader.cpp
    KXmlWriter.cpp
    ZipStore.cpp
)

kde4_add_library(kodf SHARED ${kodf_LIB_SRCS})

target_link_libraries(kodf ${KDE4_KIO_LIBS})
target_link_libraries(kodf LINK_INTERFACE_LIBRARIES ${KDE4_KIO_LIBS})
if( QCA2_FOUND )
    target_link_libraries(kodf ${QCA2_LIBRARIES})
endif( QCA2_FOUND )

set_target_properties(kodf PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kodf ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    kodf_export.h
    KOdf.h
    KOdfXmlNS.h
    KOdfGenericStyle.h
    KOdfGenericStyles.h
    KOdfFontData.h
    KOdfLoadingContext.h
    KOdfSettings.h
    KOdfStylesReader.h
    KOdfPasteBase.h
    KOdfStoreReader.h
    KOdfWriteStore.h
    KOdfStyleStack.h
    KOdfDocumentBase.h
    KOdfGenericChange.h
    KOdfGenericChanges.h
    KOdfEmbeddedDocumentSaver.h
    KOdfBorders.h
    KOdfPageLayoutData.h
    KOdfPageFormat.h
    KOdfColumnData.h
    KOdfStore.h
    KOdfStorageDevice.h
    KXmlReader.h
    KXmlWriter.h
    KProperties.h
    KUnit.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

