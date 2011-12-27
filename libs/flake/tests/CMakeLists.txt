set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${FLAKE_INCLUDES} )

########### next target ###############

set(TestPosition_test_SRCS TestPosition.cpp )
kde4_add_unit_test(TestPosition TESTNAME libs-flake-TestPosition ${TestPosition_test_SRCS})
target_link_libraries(TestPosition  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSelection_test_SRCS TestSelection.cpp )
kde4_add_unit_test(TestSelection TESTNAME libs-flake-TestSelection ${TestSelection_test_SRCS})
target_link_libraries(TestSelection  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPathShape_test_SRCS TestPathShape.cpp )
kde4_add_unit_test(TestPathShape TESTNAME libs-flake-TestPathShape ${TestPathShape_test_SRCS})
target_link_libraries(TestPathShape  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPathTool_test_SRCS TestPathTool.cpp )
kde4_add_unit_test(TestPathTool TESTNAME libs-flake-TestPathTool ${TestPathTool_test_SRCS})
target_link_libraries(TestPathTool  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestControlPointMoveCommand_test_SRCS TestControlPointMoveCommand.cpp )
kde4_add_unit_test(TestControlPointMoveCommand TESTNAME libs-flake-TestControlPointMoveCommand ${TestControlPointMoveCommand_test_SRCS})
target_link_libraries(TestControlPointMoveCommand  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPointTypeCommand_test_SRCS TestPointTypeCommand.cpp )
kde4_add_unit_test(TestPointTypeCommand TESTNAME libs-flake-TestPointTypeCommand ${TestPointTypeCommand_test_SRCS})
target_link_libraries(TestPointTypeCommand  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPointRemoveCommand_test_SRCS TestPointRemoveCommand.cpp )
kde4_add_unit_test(TestPointRemoveCommand TESTNAME libs-flake-TestPointRemoveCommand  ${TestPointRemoveCommand_test_SRCS})
target_link_libraries(TestPointRemoveCommand  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestRemoveSubpathCommand_test_SRCS TestRemoveSubpathCommand.cpp )
kde4_add_unit_test(TestRemoveSubpathCommand TESTNAME libs-flake-TestRemoveSubpathCommand ${TestRemoveSubpathCommand_test_SRCS})
target_link_libraries(TestRemoveSubpathCommand  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestShapeAt_test_SRCS TestShapeAt.cpp )
kde4_add_unit_test(TestShapeAt TESTNAME libs-flake-TestShapeAt ${TestShapeAt_test_SRCS})
target_link_libraries(TestShapeAt  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestShapePaint_test_SRCS TestShapePainting.cpp )
kde4_add_unit_test(TestShapePainting TESTNAME libs-flake-TestShapePainting ${TestShapePaint_test_SRCS})
target_link_libraries(TestShapePainting  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoShapeFactory_test_SRCS TestKoShapeFactory.cpp )
kde4_add_unit_test(TestKoShapeFactory TESTNAME libs-flake-TestKoShapeFactory ${TestKoShapeFactory_test_SRCS})
target_link_libraries(TestKoShapeFactory  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoShapeRegistry_test_SRCS TestKoShapeRegistry.cpp )
kde4_add_unit_test(TestKoShapeRegistry TESTNAME libs-flake-TestKoShapeRegistry  ${TestKoShapeRegistry_test_SRCS})
target_link_libraries(TestKoShapeRegistry  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### end ###############

set(TestShapeContainer_test_SRCS TestShapeContainer.cpp )
kde4_add_unit_test(TestShapeContainer TESTNAME libs-flake-TestShapeContainer  ${TestShapeContainer_test_SRCS})
target_link_libraries(TestShapeContainer  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### end ###############

set(TestShapeGroupCommand_test_SRCS TestShapeGroupCommand.cpp )
kde4_add_unit_test(TestShapeGroupCommand TESTNAME libs-flake-TestShapeGroupCommand  ${TestShapeGroupCommand_test_SRCS})
target_link_libraries(TestShapeGroupCommand  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### end ###############

set(TestShapeReorderCommand_test_SRCS TestShapeReorderCommand.cpp )
kde4_add_unit_test(TestShapeReorderCommand TESTNAME libs-flake-TestShapeReorderCommand  ${TestShapeReorderCommand_test_SRCS})
target_link_libraries(TestShapeReorderCommand  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPathSegment_test_SRCS TestPathSegment.cpp )
kde4_add_unit_test(TestPathSegment TESTNAME libs-flake-TestPathSegment ${TestPathSegment_test_SRCS})
target_link_libraries(TestPathSegment  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestImageCollection_test_SRCS TestImageCollection.cpp )
kde4_add_unit_test(TestImageCollection TESTNAME libs-flake-TestImageCollection ${TestImageCollection_test_SRCS})
target_link_libraries(TestImageCollection  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestCanvasResourceProvider_test_SRCS TestResourceManager.cpp )
kde4_add_unit_test(TestResourceManager TESTNAME libs-flake-TestResourceManager ${TestCanvasResourceProvider_test_SRCS})
target_link_libraries(TestResourceManager  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestShapeBackgroundCommand_test_SRCS TestShapeBackgroundCommand.cpp )
kde4_add_unit_test(TestShapeBackgroundCommand TESTNAME libs-flake-TestShapeBackgroundCommand  ${TestShapeBackgroundCommand_test_SRCS})
target_link_libraries(TestShapeBackgroundCommand  ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestShapeBorderCommand_test_SRCS TestShapeBorderCommand.cpp )
kde4_add_unit_test(TestShapeBorderCommand TESTNAME libs-flake-TestShapeBorderCommand  ${TestShapeBorderCommand_test_SRCS})
target_link_libraries(TestShapeBorderCommand ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestShapeShadowCommand_test_SRCS TestShapeShadowCommand.cpp )
kde4_add_unit_test(TestShapeShadowCommand TESTNAME libs-flake-TestShapeShadowCommand  ${TestShapeShadowCommand_test_SRCS})
target_link_libraries(TestShapeShadowCommand ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSegmentTypeCommand_test_SRCS TestSegmentTypeCommand.cpp )
kde4_add_unit_test(TestSegmentTypeCommand TESTNAME libs-flake-TestSegmentTypeCommand  ${TestSegmentTypeCommand_test_SRCS})
target_link_libraries(TestSegmentTypeCommand ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPointMergeCommand_test_SRCS TestPointMergeCommand.cpp )
kde4_add_unit_test(TestPointMergeCommand TESTNAME libs-flake-TestPointMergeCommand  ${TestPointMergeCommand_test_SRCS})
target_link_libraries(TestPointMergeCommand ${KDE4_KDECORE_LIBS} kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestConnection_SRCS TestConnection.cpp )
kde4_add_unit_test(TestConnection TESTNAME libs-flake-TestConnection  ${TestConnection_SRCS})
target_link_libraries(TestConnection kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoGenStyles_SRCS TestKoGenStyles.cpp )
kde4_add_unit_test(TestKoGenStyles TESTNAME libs-flake-TestKoGenStyles ${TestKoGenStyles_SRCS})
target_link_libraries(TestKoGenStyles kflake ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOdfSettings_SRCS TestOdfSettings.cpp )
kde4_add_unit_test(TestOdfSettings TESTNAME libs-flake-TestOdfSettings ${TestOdfSettings_SRCS})
target_link_libraries(TestOdfSettings kflake ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOdfStore_SRCS TestOdfStore.cpp )
kde4_add_unit_test(TestOdfStore TESTNAME libs-flake-TestOdfStore ${TestOdfStore_SRCS})
target_link_libraries(TestOdfStore kflake ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKodfLoadingContext_SRCS TestKOdfLoadingContext.cpp )
kde4_add_unit_test(TestKodfLoadingContext TESTNAME libs-flake-TestKodfLoadingContext ${TestKodfLoadingContext_SRCS})
target_link_libraries(TestKodfLoadingContext kflake ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(xmlwritertest_SRCS TestXmlWriter.cpp )

kde4_add_unit_test(TestXmlWriter TESTNAME libs-flake-testxmlwriter ${xmlwritertest_SRCS})

target_link_libraries(TestXmlWriter kflake ${QT_QTTEST_LIBRARY}  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(xmlreadertest_SRCS TestXmlReader.cpp )
kde4_add_unit_test(TestXmlReader TESTNAME libs-flake-testxmlreader ${xmlreadertest_SRCS})
target_link_libraries(TestXmlReader kflake ${QT_QTTEST_LIBRARY}  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

kde4_add_executable(storedroptest TEST ${storedroptest_SRCS})

target_link_libraries(storedroptest kflake  ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kxmldomtest_SRCS KXmlDomTest.cpp )

kde4_add_unit_test(KXmlDomTest TESTNAME libs-flake-kxmldomtest ${kxmldomtest_SRCS})

target_link_libraries(KXmlDomTest kflake ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} )

########### next target ###############

set(storageTest_SRCS TestStorage.cpp )

kde4_add_unit_test(TestStorage TESTNAME libs-flake-storage ${storageTest_SRCS})

target_link_libraries(TestStorage kflake ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kopropertiestest_SRCS KPropertiesTest.cpp )
kde4_add_unit_test(KPropertiesTest TESTNAME libs-flake-KPropertiesTest ${kopropertiestest_SRCS})
target_link_libraries(KPropertiesTest  ${KDE4_KDEUI_LIBS} kflake ${QT_QTTEST_LIBRARY})

set(TestKoUnit_SRCS TestKUnit.cpp)
kde4_add_unit_test(TestKUnit TESTNAME libs-flake-TestKUnit ${TestKoUnit_SRCS})
target_link_libraries(TestKUnit kflake ${QT_QTTEST_LIBRARY})

########### end ###############
