project(artwork)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-artwork.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-artwork.h )

include_directories(
    ${QT_INCLUDES}
    ${KDE4_INCLUDE_DIR}
    ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets/resources
    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets
    ${CMAKE_SOURCE_DIR}/artwork/ui
    ${CMAKE_SOURCE_DIR}/artwork/common
    ${CMAKE_SOURCE_DIR}/artwork/common/commands
)

add_subdirectory( common )
add_subdirectory( ui )
add_subdirectory( plugins )
add_subdirectory( data )
add_subdirectory( pics )
add_subdirectory( templates )

########### next target ###############

set(artworkpart_PART_SRCS artwork_factory_init.cpp)

kde4_add_plugin(artworkpart ${artworkpart_PART_SRCS})

target_link_libraries(artworkpart artworkui)

install(TARGETS artworkpart  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(artwork_KDEINIT_SRCS main.cpp)

kde4_add_app_icon( artwork_KDEINIT_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-app-artwork.png" )
kde4_add_kdeinit_executable( artwork ${artwork_KDEINIT_SRCS})

if (Q_WS_MAC)
   set_target_properties(artwork PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
   set_target_properties(artwork PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.koffice.artwork")
   set_target_properties(artwork PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Artwork 2")
endif (Q_WS_MAC)

target_link_libraries(kdeinit_artwork artworkui)

install(TARGETS kdeinit_artwork  ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries(artwork kdeinit_artwork)
install(TARGETS artwork  ${INSTALL_TARGETS_DEFAULT_ARGS})

