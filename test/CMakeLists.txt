add_library(TestUtils TestUtils.cpp)

include_directories(../third_party/Catch/single_include)

add_executable(TestPager TestPager.cpp)
target_link_libraries(TestPager TestUtils Pager Page)

add_executable(TestTableManager TestTableManager.cpp)
target_link_libraries(TestTableManager Pager Page)