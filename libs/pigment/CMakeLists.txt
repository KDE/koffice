project(pigmentcms)

include_directories( ${PIGMENT_INCLUDES} ${QT_INCLUDES} )

set(FILE_OPENEXR_SOURCES)
set(LINK_OPENEXR_LIB)
if(OPENEXR_FOUND)
    include_directories(${OPENEXR_INCLUDE_DIR})
    set(LINK_OPENEXR_LIB ${OPENEXR_LIBRARIES})
    add_definitions(${OPENEXR_DEFINITIONS})
endif(OPENEXR_FOUND)

add_subdirectory(tests)
add_subdirectory(benchmarks)

set(pigmentcms_SRCS
    PluginLoader.cpp
    KoBasicHistogramProducers.cpp
    KoColor.cpp
    KoColorConversionAlphaTransformation.cpp
    KoColorConversionCache.cpp
    KoColorConversions.cpp
    KoColorConversionSystem.cpp
    KoColorConversionTransformation.cpp
    KoColorConversionTransformationFactory.cpp
    KoColorModelStandardIds.cpp
    KoColorProfile.cpp
    KoColorSpace.cpp
    KoColorSpaceEngine.cpp
    KoColorSpaceFactory.cpp
    KoColorSpaceMaths.cpp
    KoColorSpaceRegistry.cpp
    KoColorTransformation.cpp
    KoColorTransformationFactory.cpp
    KoColorTransformationFactoryRegistry.cpp
    KoCompositeOp.cpp
    KoCopyColorConversionTransformation.cpp
    KoFallBackColorTransformation.cpp
    KoHistogramProducer.cpp
    KoMultipleColorConversionTransformation.cpp
    KoUniqueNumberForIdServer.cpp
    colorspaces/KoAlphaColorSpace.cpp
    colorspaces/KoLabColorSpace.cpp
    colorspaces/KoRgbU16ColorSpace.cpp
    colorspaces/KoRgbU8ColorSpace.cpp
    colorspaces/KoSimpleColorSpaceEngine.cpp
    colorprofiles/KoDummyColorProfile.cpp
    resources/KoAbstractGradient.cpp
    resources/KoColorSet.cpp
    resources/KoPattern.cpp
    resources/KoResource.cpp
    resources/KoStopGradient.cpp
    resources/KoSegmentGradient.cpp
)

set (EXTRA_LIBRARIES ${LINK_OPENEXR_LIB})

if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

kde4_add_library(pigmentcms SHARED ${pigmentcms_SRCS})

target_link_libraries(
    pigmentcms
    ${EXTRA_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTXML_LIBRARY}
)

target_link_libraries(
    pigmentcms LINK_INTERFACE_LIBRARIES ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${EXTRA_LIBRARIES}
)

set_target_properties(pigmentcms PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS pigmentcms  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    KoBasicHistogramProducers.h
    KoColor.h
    KoColorConversionSystem.h
    KoColorConversionTransformation.h
    KoColorConversionTransformationAbstractFactory.h
    KoColorConversionTransformationFactory.h
    KoColorConversions.h
    KoColorModelStandardIds.h
    KoColorProfile.h
    KoColorSpace.h
    KoColorSpaceEngine.h
    KoColorSpaceFactory.h
    KoColorSpaceMaths.h
    KoColorSpaceRegistry.h
    KoColorTransformation.h
    KoColorTransformationFactory.h
    KoColorTransformationFactoryRegistry.h
    KoCompositeOp.h
    KoFallBackColorTransformation.h
    KoHistogramProducer.h
    KoMultipleColorConversionTransformation.h
    KoUniqueNumberForIdServer.h
    colorspaces/KoAlphaColorSpace.h
    pigment_export.h
    resources/KoAbstractGradient.h
    resources/KoColorSet.h
    resources/KoPattern.h
    resources/KoResource.h
    resources/KoSegmentGradient.h
    resources/KoStopGradient.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES pigment.desktop pigmentextension.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
