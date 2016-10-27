add_library(TestUtils TestUtils.cpp)

include_directories(../third_party/Catch/single_include)

add_executable(TestPager TestPager.cpp)
target_link_libraries(TestPager TestUtils Pager Page)
#
add_executable(TestTableManager TestTableManager.cpp)
target_link_libraries(TestTableManager TableManager Pager Page)

add_executable(TestRecordManager TestRecordManager.cpp)
target_link_libraries(TestRecordManager RecordManager TableManager Pager Page FileUtils TestUtils)

add_executable(TestLexer TestLexer.cpp)
target_link_libraries(TestLexer SQLParser TestUtils)

add_executable(TestParser TestParser.cpp)
target_link_libraries(TestParser AST SQLParser TestUtils)
