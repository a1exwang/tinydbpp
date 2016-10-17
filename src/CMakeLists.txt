add_library(FileUtils FileUtils.cpp )

add_library(Pager Pager.cpp )
target_link_libraries(Pager FileUtils)
add_library(Page Page.cpp)
target_link_libraries(Page FileUtils)

add_subdirectory(RecordManage)