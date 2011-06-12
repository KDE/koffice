add_subdirectory( tests )

include_directories( ${KOODF_INCLUDES} )

########### libkoodf ###############

if( QCA2_FOUND )
    include_directories( ${QCA2_INCLUDE_DIR} )
    add_definitions( -DQCA2 )
endif( QCA2_FOUND )

set(koodf_LIB_SRCS
    KOdfBorders.cpp
    DirectoryStore.cpp
    KOdfEmbeddedDocumentSaver.cpp
    EncryptedStore.cpp
    KoEncryptionChecker.cpp
    KOdfFontData.cpp
    KOdfGenericChange.cpp
    KOdfGenericChanges.cpp
    KOdfGenericStyle.cpp
    KOdfGenericStyles.cpp
    KoOdfSettings.cpp
    KoOdf.cpp
    KOdfDocumentBase.cpp
    KoOdfGraphicStyles.cpp
    KOdfLoadingContext.cpp
    KoOdfNumberStyles.cpp
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

kde4_add_library(koodf SHARED ${koodf_LIB_SRCS})

target_link_libraries(koodf ${KDE4_KIO_LIBS})
target_link_libraries(koodf LINK_INTERFACE_LIBRARIES ${KDE4_KIO_LIBS})
if( QCA2_FOUND )
    target_link_libraries(koodf ${QCA2_LIBRARIES})
endif( QCA2_FOUND )

set_target_properties(koodf PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koodf ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    koodf_export.h
    KoOdf.h
    KOdfXmlNS.h
    KOdfGenericStyle.h
    KOdfGenericStyles.h
    KOdfFontData.h
    KOdfLoadingContext.h
    KoOdfSettings.h
    KOdfStylesReader.h
    KoOdfNumberStyles.h
    KOdfPasteBase.h
    KOdfStoreReader.h
    KOdfWriteStore.h
    KOdfStyleStack.h
    KoOdfGraphicStyles.h
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

