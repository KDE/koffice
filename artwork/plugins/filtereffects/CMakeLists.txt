include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} )

set(artworkfiltereffects_PART_SRCS
    ArtworkFilterEffectsPlugin.cpp
    BlurEffect.cpp
    BlurEffectFactory.cpp
    BlurEffectConfigWidget.cpp
    OffsetEffect.cpp
    OffsetEffectFactory.cpp
    OffsetEffectConfigWidget.cpp
    MergeEffect.cpp
    MergeEffectFactory.cpp
    MergeEffectConfigWidget.cpp
    ColorMatrixEffect.cpp
    ColorMatrixEffectFactory.cpp
    ColorMatrixEffectConfigWidget.cpp
    FloodEffect.cpp
    FloodEffectFactory.cpp
    FloodEffectConfigWidget.cpp
    CompositeEffect.cpp
    CompositeEffectFactory.cpp
    CompositeEffectConfigWidget.cpp
    BlendEffect.cpp
    BlendEffectFactory.cpp
    BlendEffectConfigWidget.cpp
    ComponentTransferEffect.cpp
    ComponentTransferEffectFactory.cpp
    ComponentTransferEffectConfigWidget.cpp
    ImageEffect.cpp
    ImageEffectFactory.cpp
    ImageEffectConfigWidget.cpp
    MorphologyEffect.cpp
    MorphologyEffectFactory.cpp
    MorphologyEffectConfigWidget.cpp
    ConvolveMatrixEffect.cpp
    ConvolveMatrixEffectFactory.cpp
    ConvolveMatrixEffectConfigWidget.cpp
    MatrixDataModel.cpp
    )

kde4_add_plugin(artworkfiltereffects ${artworkfiltereffects_PART_SRCS})

target_link_libraries(artworkfiltereffects kflake kowidgets)

install(TARGETS artworkfiltereffects  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES artworkfiltereffects.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
