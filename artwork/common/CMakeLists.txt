set(artworkcommon_LIB_SRCS 
    ArtworkDocument.cpp
    ArtworkGlobal.cpp
    ArtworkGradientHelper.cpp
    ArtworkOutlinePaintingStrategy.cpp

    commands/ArtworkLayerReorderCommand.cpp
    commands/ArtworkBooleanCommand.cpp
)

kde4_add_library( artworkcommon SHARED ${artworkcommon_LIB_SRCS} )

target_link_libraries(artworkcommon komain kowidgets)
target_link_libraries(artworkcommon LINK_INTERFACE_LIBRARIES komain)

set_target_properties(artworkcommon PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS artworkcommon  ${INSTALL_TARGETS_DEFAULT_ARGS})
