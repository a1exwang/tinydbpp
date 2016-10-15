add_library(FileUtils FileUtils.cpp)

add_library(Pager Pager.cpp)
target_link_libraries(Pager FileUtils)
add_library(Page Page.cpp)
target_link_libraries(Page FileUtils)

add_library(RecordManage RecordManage/RecodeManager.cpp RecordManage/RecodeManager.h RecordManage/TableManager.cpp RecordManage/TableManager.h RecordManage/Checker.cpp)
target_link_libraries(RecordManage Pager)