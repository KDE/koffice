include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(kpr_pageeffect_clockwipe_SRCS
    Plugin.cpp
    SCClockWipeStrategy.cpp
    SCCenterFanWipeStrategy.cpp
    SCSideFanWipeStrategy.cpp
    SCClockWipeSubpathHelper.cpp
    SCSweepWipeStrategy.cpp

    clockwipe/SCClockWipeEffectFactory.cpp
    pinwheelwipe/SCPinWheelWipeEffectFactory.cpp
    singlesweepwipe/SCSingleSweepWipeEffectFactory.cpp
    singlesweepwipe/SCSingleSweepWipeStrategy.cpp
    fanwipe/SCFanWipeEffectFactory.cpp
    doublefanwipe/SCDoubleFanWipeEffectFactory.cpp
    doublesweepwipe/SCDoubleSweepWipeEffectFactory.cpp
    doublesweepwipe/SCDoubleSweepWipeStrategy.cpp
    saloondoorwipe/SCSaloonDoorWipeEffectFactory.cpp
    saloondoorwipe/SCSaloonDoorWipeStrategy.cpp
    windshieldwipe/SCWindShieldWipeEffectFactory.cpp
    windshieldwipe/SCWindShieldWipeStrategy.cpp
    )

kde4_add_plugin(kpr_pageeffect_clockwipe ${kpr_pageeffect_clockwipe_SRCS})

target_link_libraries(
    kpr_pageeffect_clockwipe 
    showcaseprivate 
    flake
    
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

install(TARGETS kpr_pageeffect_clockwipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kpr_pageeffect_clockwipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


