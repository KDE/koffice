set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KODFTEXT_INCLUDES} )
add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")

if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)

########### next target ###############

set(TestDocumentLayout_test_SRCS TestDocumentLayout.cpp)
kde4_add_unit_test(TestDocumentLayout1 TESTNAME libs-kotext-TestDocumentLayout  ${TestDocumentLayout_test_SRCS})
target_link_libraries(TestDocumentLayout1 kotext komain ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestBorder_test_SRCS TestBorder.cpp)
kde4_add_unit_test(TestBorder TESTNAME libs-kotext-TestBorder  ${TestBorder_test_SRCS})
target_link_libraries(TestBorder kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestKoTableColumnAndRowStyleManager_test_SRCS TestKoTableColumnAndRowStyleManager.cpp)
kde4_add_unit_test(TestKoTableColumnAndRowStyleManager TESTNAME libs-kotext-KTableColumnAndRowStyleManager ${TestKoTableColumnAndRowStyleManager_test_SRCS})
target_link_libraries(TestKoTableColumnAndRowStyleManager kotext  ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestStyles_test_SRCS TestStyles.cpp)
kde4_add_unit_test(TestStyles TESTNAME libs-kotext-styles-TestStyles  ${TestStyles_test_SRCS})
target_link_libraries(TestStyles kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestListStyly_test_SRCS TestListStyle.cpp)
kde4_add_unit_test(TestListStyle TESTNAME libs-kotext-styles-TestListStyle  ${TestListStyly_test_SRCS})
target_link_libraries(TestListStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSectionStyle_test_SRCS TestSectionStyle.cpp)
kde4_add_unit_test(TestSectionStyle TESTNAME libs-kotext-styles-TestSectionStyle ${TestSectionStyle_test_SRCS})
target_link_libraries(TestSectionStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestTableCellStyle_test_SRCS TestTableCellStyle.cpp)
kde4_add_unit_test(TestTableCellStyle TESTNAME libs-kotext-styles-TestTableCellStyle ${TestTableCellStyle_test_SRCS})
target_link_libraries(TestTableCellStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestLoadStyle_test_SRCS TestLoadStyle.cpp)
kde4_add_unit_test(TestLoadStyle TESTNAME kotext-odf-TestLoadStyle ${TestLoadStyle_test_SRCS})
target_link_libraries(TestLoadStyle kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestLoading_test_SRCS TestLoading.cpp)
kde4_add_unit_test(TestLoading TESTNAME kotext-odf-TestLoading ${TestLoading_test_SRCS})
target_link_libraries(TestLoading kotext ${QT_QTTEST_LIBRARY}  ${QT_QTSCRIPT_LIBRARY})

########### next target ###############

set(normalizeWhiteSpace_SRCS tst_normalizeWhiteSpace.cpp)
kde4_add_executable(normalizeWhiteSpace ${normalizeWhiteSpace_SRCS})
target_link_libraries(normalizeWhiteSpace kotext ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestChangeTracking_test_SRCS TestChangeTracking.cpp)
kde4_add_unit_test(TestChangeTracking TESTNAME kotext-odf-TestChangeTracking ${TestChangeTracking_test_SRCS})
target_link_libraries(TestChangeTracking kotext ${QT_QTTEST_LIBRARY})

