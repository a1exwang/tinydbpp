include_directories(.)
add_library(TableManager TableManager.cpp)
target_link_libraries(TableManager Pager Page FileUtils)

add_library(RecordManager RecordManager.cpp)
target_link_libraries(RecordManager Pager Page FileUtils)
