set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/showcase/part ${KOMAIN_INCLUDES} )

########### next target ###############

#set(TestShapeAnimations_test_SRCS TestShapeAnimations.cpp )

#kde4_add_unit_test(TestShapeAnimations TESTNAME showcase-part-TestShapeAnimations  ${TestShapeAnimations_test_SRCS})

#target_link_libraries(TestShapeAnimations showcaseprivate ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestCustomSlideShows_test_SRCS TestCustomSlideShows.cpp )

kde4_add_unit_test(TestCustomSlideShows TESTNAME showcase-part-TestCustomSlideShows  ${TestCustomSlideShows_test_SRCS})

target_link_libraries(TestCustomSlideShows showcaseprivate ${QT_QTTEST_LIBRARY})

