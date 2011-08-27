set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KODF_INCLUDES} )

########### next target ###############

set(TestKoGenStyles_SRCS TestKoGenStyles.cpp )
kde4_add_unit_test(TestKoGenStyles TESTNAME libs-odf-TestKoGenStyles ${TestKoGenStyles_SRCS})
target_link_libraries(TestKoGenStyles kodf ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOdfSettings_SRCS TestOdfSettings.cpp )
kde4_add_unit_test(TestOdfSettings TESTNAME libs-odf-TestOdfSettings ${TestOdfSettings_SRCS})
target_link_libraries(TestOdfSettings kodf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOdfStore_SRCS TestOdfStore.cpp )
kde4_add_unit_test(TestOdfStore TESTNAME libs-odf-TestOdfStore ${TestOdfStore_SRCS})
target_link_libraries(TestOdfStore kodf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKodfLoadingContext_SRCS TestKOdfLoadingContext.cpp )
kde4_add_unit_test(TestKodfLoadingContext TESTNAME libs-odf-TestKodfLoadingContext ${TestKodfLoadingContext_SRCS})
target_link_libraries(TestKodfLoadingContext kodf ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(xmlwritertest_SRCS TestXmlWriter.cpp )

kde4_add_unit_test(TestXmlWriter TESTNAME libs-store-testxmlwriter ${xmlwritertest_SRCS})

target_link_libraries(TestXmlWriter kodf ${QT_QTTEST_LIBRARY}  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(xmlreadertest_SRCS TestXmlReader.cpp )
kde4_add_unit_test(TestXmlReader TESTNAME libs-store-testxmlreader ${xmlreadertest_SRCS})
target_link_libraries(TestXmlReader kodf ${QT_QTTEST_LIBRARY}  ${KDE4_KDEUI_LIBS})

########### next target ###############

set(storedroptest_SRCS storedroptest.cpp )

kde4_add_executable(storedroptest TEST ${storedroptest_SRCS})

target_link_libraries(storedroptest kodf  ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kodomtest_SRCS kodomtest.cpp )

kde4_add_unit_test(kodomtest TESTNAME libs-store-kodomtest ${kodomtest_SRCS})

target_link_libraries(kodomtest kodf ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} )

########### next target ###############

set(storageTest_SRCS TestStorage.cpp )

kde4_add_unit_test(TestStorage TESTNAME libs-store-storage ${storageTest_SRCS})

target_link_libraries(TestStorage kodf ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})

########### next target ###############

set(kopropertiestest_SRCS KPropertiesTest.cpp )
kde4_add_unit_test(KPropertiesTest TESTNAME libs-kobase-KPropertiesTest ${kopropertiestest_SRCS})
target_link_libraries(KPropertiesTest  ${KDE4_KDEUI_LIBS} kodf ${QT_QTTEST_LIBRARY})

set(TestKoUnit_SRCS TestKUnit.cpp)
kde4_add_unit_test(TestKUnit TESTNAME libs-kobase-TestKUnit ${TestKoUnit_SRCS})
target_link_libraries(TestKUnit kodf ${QT_QTTEST_LIBRARY})

########### end ###############
