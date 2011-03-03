include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(kpr_pageeffect_matrixwipe_SRCS
    Plugin.cpp
    SCMatrixWipeStrategy.cpp

    snakewipe/SCSnakeWipeEffectFactory.cpp
    snakewipe/SCSnakeWipeBottomRightHorizontalStrategy.cpp
    snakewipe/SCSnakeWipeBottomRightVerticalStrategy.cpp
    snakewipe/SCSnakeWipeTopLeftHorizontalStrategy.cpp
    snakewipe/SCSnakeWipeTopLeftVerticalStrategy.cpp
    snakewipe/SCSnakeWipeDiagonalStrategy.cpp
    snakewipe/SCSnakeWipeTopLeftDiagonalStrategy.cpp
    snakewipe/SCSnakeWipeTopRightDiagonalStrategy.cpp
    snakewipe/SCSnakeWipeBottomRightDiagonalStrategy.cpp
    snakewipe/SCSnakeWipeBottomLeftDiagonalStrategy.cpp

    spiralwipe/SCSpiralWipeEffectFactory.cpp
    spiralwipe/SCSpiralWipeStrategy.cpp

    parallelsnakes/SCParallelSnakesWipeEffectFactory.cpp
    parallelsnakes/SCParallelSnakesWipeHorizontalStrategy.cpp
    parallelsnakes/SCParallelSnakesWipeVerticalStrategy.cpp
    parallelsnakes/SCParallelSnakesWipeDiagonalStrategy.cpp

    boxsnakes/SCBoxSnakesWipeEffectFactory.cpp
    boxsnakes/SCBoxSnakesWipeStrategy.cpp

    waterfallwipe/SCWaterfallWipeEffectFactory.cpp
    waterfallwipe/SCWaterfallWipeTopLeftStrategy.cpp
    waterfallwipe/SCWaterfallWipeTopRightStrategy.cpp
    waterfallwipe/SCWaterfallWipeBottomLeftStrategy.cpp
    waterfallwipe/SCWaterfallWipeBottomRightStrategy.cpp
)

kde4_add_plugin(kpr_pageeffect_matrixwipe ${kpr_pageeffect_matrixwipe_SRCS})

target_link_libraries(
    kpr_pageeffect_matrixwipe
    showcaseprivate 
    flake 
    
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

install(TARGETS kpr_pageeffect_matrixwipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kpr_pageeffect_matrixwipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


