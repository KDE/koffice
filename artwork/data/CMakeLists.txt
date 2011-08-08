install( FILES  simple.kgr allcolors.kgr  DESTINATION ${DATA_INSTALL_DIR}/artwork/gradients)
install( FILES  artwork_module.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES  artworkpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  artwork.rc artwork_readonly.rc  DESTINATION ${DATA_INSTALL_DIR}/artwork)
install( PROGRAMS  artwork.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  artworkrc DESTINATION ${CONFIG_INSTALL_DIR} )

