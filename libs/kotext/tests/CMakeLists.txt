set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KODFTEXT_INCLUDES} )
add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

########### next target ###############

set(TestDocumentLayout_test_SRCS TestDocumentLayout.cpp)
kde4_add_unit_test(TestDocumentLayout1 TESTNAME libs-kodftext-TestDocumentLayout  ${TestDocumentLayout_test_SRCS})
target_link_libraries(TestDocumentLayout1 kodftext komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
kde4_add_unit_test(TestBorder TESTNAME libs-kodftext-TestBorder  ${TestBorder_test_SRCS})
target_link_libraries(TestBorder kodftext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
kde4_add_unit_test(TestKoTableColumnAndRowStyleManager TESTNAME libs-kodftext-KTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
target_link_libraries(TestKoTableColumnAndRowStyleManager kodftext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestStyles_test_SRCS TestStyles.cpp)
kde4_add_unit_test(TestStyles TESTNAME libs-kodftext-styles-TestStyles  ${TestStyles_test_SRCS})
target_link_libraries(TestStyles kodftext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestListStyly_test_SRCS TestListStyle.cpp)
kde4_add_unit_test(TestListStyle TESTNAME libs-kodftext-styles-TestListStyle  ${TestListStyly_test_SRCS})
target_link_libraries(TestListStyle kodftext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSectionStyle_test_SRCS TestSectionStyle.cpp)
kde4_add_unit_test(TestSectionStyle TESTNAME libs-kodftext-styles-TestSectionStyle ${TestSectionStyle_test_SRCS})
target_link_libraries(TestSectionStyle kodftext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestTableCellStyle_test_SRCS TestTableCellStyle.cpp)
kde4_add_unit_test(TestTableCellStyle TESTNAME libs-kodftext-styles-TestTableCellStyle ${TestTableCellStyle_test_SRCS})
target_link_libraries(TestTableCellStyle kodftext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestLoadStyle_test_SRCS TestLoadStyle.cpp)
kde4_add_unit_test(TestLoadStyle TESTNAME kodftext-odf-TestLoadStyle ${TestLoadStyle_test_SRCS})
target_link_libraries(TestLoadStyle kodftext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestLoading_test_SRCS TestLoading.cpp)
kde4_add_unit_test(TestLoading TESTNAME kodftext-odf-TestLoading ${TestLoading_test_SRCS})
target_link_libraries(TestLoading kodftext ${QT_QTTEST_LIBRARY}  ${QT_QTSCRIPT_LIBRARY})

########### next target ###############

set(normalizeWhiteSpace_SRCS tst_normalizeWhiteSpace.cpp)
kde4_add_executable(normalizeWhiteSpace ${normalizeWhiteSpace_SRCS})
target_link_libraries(normalizeWhiteSpace kodftext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestChangeTracking_test_SRCS TestChangeTracking.cpp)
kde4_add_unit_test(TestChangeTracking TESTNAME kodftext-odf-TestChangeTracking ${TestChangeTracking_test_SRCS})
target_link_libraries(TestChangeTracking kodftext ${QT_QTTEST_LIBRARY})

