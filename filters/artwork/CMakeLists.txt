set(SHAPES_SOURCES
    ${CMAKE_SOURCE_DIR}/plugins/pathshapes/ellipse/EllipseShape.cpp
    ${CMAKE_SOURCE_DIR}/plugins/pathshapes/rectangle/RectangleShape.cpp
    ${CMAKE_SOURCE_DIR}/plugins/pathshapes/star/StarShape.cpp
    ${CMAKE_SOURCE_DIR}/plugins/artistictextshape/ArtisticTextShape.cpp
    ${CMAKE_SOURCE_DIR}/plugins/artistictextshape/ArtisticTextShapeLoadingUpdater.cpp)


include_directories(
        ${KODFTEXT_INCLUDES}
        ${KOMAIN_INCLUDES}
        ${FLAKE_INCLUDES}
        ${CMAKE_SOURCE_DIR}/artwork
        ${CMAKE_SOURCE_DIR}/artwork/common
        ${CMAKE_SOURCE_DIR}/artwork/ui
        ${CMAKE_BINARY_DIR}/artwork
        ${KDE4_INCLUDES}
        ${CMAKE_SOURCE_DIR}/plugins/ )

macro_optional_find_package(WPD)
macro_optional_find_package(WPG)
macro_log_feature(LIBWPG_FOUND "WPG" "WordPerfect Graphics Library" "http://libwpg.sourceforge.net/" FALSE "" "Required by the Artwork WPG import filter")
macro_log_feature(LIBWPG_STREAM_INCLUDE_DIR "WPS" "WordPerfect Graphics Stream Library" "http://libwps.sourceforge.net/" FALSE "" "Required by the Artwork WPG import filter")
macro_log_feature(LIBREVENGE_INCLUDE_DIR "Revenge" "WordPerfect REVerse ENGineering Library" "http://libwpd.sourceforge.net/" FALSE "" "Required by the Artwork WPG import filter")
macro_optional_find_package(Poppler)
# The pdf-importer needs the not-officially-supported XPDF Headers
# Installing these is off by default in poppler sources, so lets make
# sure they're really there before trying to build the pdf import
find_path(POPPLER_XPDF_HEADERS poppler-config.h
    HINTS ${POPPLER_INCLUDE_DIR} )
if( POPPLER_FOUND AND NOT POPPLER_XPDF_HEADERS )
    message( STATUS "Could NOT find the XPDF headers in Poppler Qt4 which are necessary for the Artwork PDF import filter")
    set( POPPLER_FOUND FALSE )
endif( POPPLER_FOUND AND NOT POPPLER_XPDF_HEADERS )
macro_log_feature( POPPLER_FOUND "poppler-qt4" "The Poppler Qt4 interface library, with XPDF headers" "http://poppler.freedesktop.org" FALSE "" "Required by the Artwork PDF import filter")

add_subdirectory( png )
add_subdirectory( svg )
if (WPD_FOUND AND LIBWPG_FOUND AND LIBWPG_STREAM_INCLUDE_DIR AND LIBREVENGE_FOUND)
  add_subdirectory( wpg )
endif (WPD_FOUND AND LIBWPG_FOUND AND LIBWPG_STREAM_INCLUDE_DIR AND LIBREVENGE_FOUND)
add_subdirectory( wmf )
add_subdirectory( artwork1.x )
add_subdirectory( eps )
if (POPPLER_FOUND)
    add_subdirectory( pdf )
endif (POPPLER_FOUND)
