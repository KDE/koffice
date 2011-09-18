include_directories(${CMAKE_SOURCE_DIR}/filters/kword/libexport ${KOMAIN_INCLUDES}  ${KODFTEXT_INCLUDES})

add_subdirectory( powerpoint )
add_subdirectory( kpr2odf )
if(NOT WIN32 AND NOT QT_MAC_USE_COCOA)
    add_subdirectory( pptx )
endif(NOT WIN32 AND NOT QT_MAC_USE_COCOA)
