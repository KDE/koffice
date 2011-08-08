
########### next target ###############

#set(artworkepsexport_PART_SRCS epsexport.cc epsexportdlg.cc )

#kde4_add_plugin(artworkepsexport ${artworkepsexport_PART_SRCS})



#target_link_libraries(artworkepsexport  ${KDE4_KDECORE_LIBS}  artworkcommon )

#install(TARGETS artworkepsexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

macro_optional_find_package(Pstoedit)
macro_log_feature(PSTOEDIT_SVG_PLUGIN_FOUND "pstoedit with svg plugin" "Translates PostScript and PDF graphics to SVG" "http://www.pstoedit.net/" FALSE "" "Required at runtime by the Artwork EPS import filter")

if (PSTOEDIT_SVG_PLUGIN_FOUND)

add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(artworkepsimport_PART_SRCS epsimport.cc pscommentlexer.cc )

kde4_add_plugin(artworkepsimport ${artworkepsimport_PART_SRCS})

target_link_libraries(artworkepsimport komain)

install(TARGETS artworkepsimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(
FILES  
#artwork_eps_export.desktop  
	artwork_eps_import.desktop
#  	artwork_ps_import.desktop
DESTINATION ${SERVICES_INSTALL_DIR})

endif (PSTOEDIT_SVG_PLUGIN_FOUND)

