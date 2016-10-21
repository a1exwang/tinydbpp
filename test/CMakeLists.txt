add_library(TestUtils TestUtils.cpp)

include_directories(../third_party/Catch/single_include)

add_executable(TestPager TestPager.cpp)
target_link_libraries(TestPager TestUtils Pager Page)
#
add_executable(TestTableManager TestTableManager.cpp)
target_link_libraries(TestTableManager TableManager Pager Page)

add_executable(TestRecordManager TestRecordManager.cpp)
target_link_libraries(TestRecordManager RecordManager TableManager Pager Page FileUtils TestUtils)

add_executable(TestBTree TestBTree.cpp)
target_link_libraries(TestBTree RecordManager TableManager BTree Pager Page FileUtils TestUtils)
