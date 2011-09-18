include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(showcase_pageeffect_clockwipe_SRCS
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

kde4_add_plugin(showcase_pageeffect_clockwipe ${showcase_pageeffect_clockwipe_SRCS})

target_link_libraries(showcase_pageeffect_clockwipe showcaseprivate)

install(TARGETS showcase_pageeffect_clockwipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_clockwipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


