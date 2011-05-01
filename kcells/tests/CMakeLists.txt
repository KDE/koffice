include(MacroAddCompileFlags)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/kcells  ${EIGEN2_INCLUDE_DIR} ${KOMAIN_INCLUDES} ${KDE4_INCLUDES} )

########### Core Functionality ###############

set(TestDependencies_SRCS TestDependencies.cpp)
kde4_add_unit_test(TestDependencies TESTNAME kcells-Dependencies ${TestDependencies_SRCS})
target_link_libraries(TestDependencies kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestFormula_SRCS TestFormula.cpp)
kde4_add_unit_test(TestFormula TESTNAME kcells-KCFormula  ${TestFormula_SRCS})
target_link_libraries(TestFormula kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestOpenFormula_SRCS TestOpenFormula.cpp)
kde4_add_unit_test(TestOpenFormula TESTNAME kcells-OpenFormula  ${TestOpenFormula_SRCS})
target_link_libraries(TestOpenFormula kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestPointStorage_SRCS TestPointStorage.cpp)
kde4_add_unit_test(TestPointStorage TESTNAME kcells-KCPointStorage  ${TestPointStorage_SRCS})
target_link_libraries(TestPointStorage kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestRegion_SRCS TestRegion.cpp)
kde4_add_unit_test(TestRegion TESTNAME kcells-Region  ${TestRegion_SRCS})
target_link_libraries(TestRegion kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestRTree_SRCS TestRTree.cpp)
kde4_add_unit_test(TestRTree TESTNAME kpsread-KCRTree  ${TestRTree_SRCS})
target_link_libraries(TestRTree ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSelection_SRCS TestSelection.cpp)
kde4_add_unit_test(TestSelection TESTNAME kcells-Selection ${TestSelection_SRCS})
target_link_libraries(TestSelection kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestValue_SRCS TestValue.cpp)
kde4_add_unit_test(TestValue TESTNAME kcells-KCValue  ${TestValue_SRCS})
target_link_libraries(TestValue kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestStyleStorage_SRCS TestStyleStorage.cpp)
kde4_add_unit_test(TestStyleStorage TESTNAME kcells-KCStyleStorage ${TestStyleStorage_SRCS})
target_link_libraries(TestStyleStorage kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})


########### KCFunction tests ###############

set(TestBitopsFunctions_SRCS TestBitopsFunctions.cpp)
kde4_add_unit_test(TestBitopsFunctions TESTNAME kcells-BitopsFunctions  ${TestBitopsFunctions_SRCS})
target_link_libraries(TestBitopsFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestDatabaseFunctions_SRCS TestDatabaseFunctions.cpp)
kde4_add_unit_test(TestDatabaseFunctions TESTNAME kcells-DatabaseFunctions  ${TestDatabaseFunctions_SRCS})
target_link_libraries(TestDatabaseFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### target TestDatetimeFunctions ###############

set(TestDatetimeFunctions_SRCS TestDatetimeFunctions.cpp)
kde4_add_unit_test(TestDatetimeFunctions TESTNAME kcells-DatetimeFunctions  ${TestDatetimeFunctions_SRCS})
target_link_libraries(TestDatetimeFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestEngineeringFunctions_SRCS TestEngineeringFunctions.cpp)
kde4_add_unit_test(TestEngineeringFunctions TESTNAME kcells-EngineeringFunctions  ${TestEngineeringFunctions_SRCS})
target_link_libraries(TestEngineeringFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestFinancialFunctions_SRCS TestFinancialFunctions.cpp)
kde4_add_unit_test(TestFinancialFunctions TESTNAME kcells-FinancialFunctions  ${TestFinancialFunctions_SRCS})
target_link_libraries(TestFinancialFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestInformationFunctions_SRCS TestInformationFunctions.cpp)
kde4_add_unit_test(TestInformationFunctions TESTNAME kcells-InformationFunctions  ${TestInformationFunctions_SRCS})
target_link_libraries(TestInformationFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestLogicFunctions_SRCS TestLogicFunctions.cpp)
kde4_add_unit_test(TestLogicFunctions TESTNAME kcells-LogicFunctions  ${TestLogicFunctions_SRCS})
target_link_libraries(TestLogicFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestMathFunctions_SRCS TestMathFunctions.cpp)
kde4_add_unit_test(TestMathFunctions TESTNAME kcells-MathFunctions  ${TestMathFunctions_SRCS})
target_link_libraries(TestMathFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestStatisticalFunctions_SRCS TestStatisticalFunctions.cpp)
kde4_add_unit_test(TestStatisticalFunctions TESTNAME kcells-StatisticalFunctions  ${TestStatisticalFunctions_SRCS})
target_link_libraries(TestStatisticalFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestTextFunctions_SRCS TestTextFunctions.cpp)
kde4_add_unit_test(TestTextFunctions TESTNAME kcells-TextFunctions  ${TestTextFunctions_SRCS})
target_link_libraries(TestTextFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestTrigFunctions_SRCS TestTrigFunctions.cpp)
kde4_add_unit_test(TestTrigFunctions TESTNAME kcells-TrigFunctions  ${TestTrigFunctions_SRCS})
target_link_libraries(TestTrigFunctions kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestValueFormatter_SRCS TestValueFormatter.cpp)
kde4_add_unit_test(TestValueFormatter TESTNAME kcells-KCValueFormatter  ${TestValueFormatter_SRCS})
macro_add_compile_flags(TestValueFormatter "-DKSPREAD_UNIT_TEST")
target_link_libraries(TestValueFormatter kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestSheet_SRCS TestSheet.cpp)
kde4_add_unit_test(TestSheet TESTNAME kcells-KCSheet  ${TestSheet_SRCS})
macro_add_compile_flags(TestSheet "-DKSPREAD_UNIT_TEST")
target_link_libraries(TestSheet kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(TestRowRepeatStorage_SRCS TestRowRepeatStorage.cpp)
kde4_add_unit_test(TestRowRepeatStorage TESTNAME kcells-KCRowRepeatStorage ${TestRowRepeatStorage_SRCS})
target_link_libraries(TestRowRepeatStorage kcellscommon ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### Benchmarks ###############

# set(BenchmarkCluster_SRCS BenchmarkCluster.cpp ../KCCluster.cpp) # explicit KCCluster.cpp for no extra symbol visibility
# kde4_add_executable(BenchmarkCluster TEST ${BenchmarkCluster_SRCS})
# target_link_libraries(BenchmarkCluster kcellscommon ${QT_QTTEST_LIBRARY})

########### next target ###############

set(BenchmarkPointStorage_SRCS BenchmarkPointStorage.cpp)
kde4_add_executable(BenchmarkPointStorage TEST ${BenchmarkPointStorage_SRCS})
target_link_libraries(BenchmarkPointStorage ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(BenchmarkRTree_SRCS BenchmarkRTree.cpp)
kde4_add_executable(BenchmarkRTree TEST ${BenchmarkRTree_SRCS})
target_link_libraries(BenchmarkRTree ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
