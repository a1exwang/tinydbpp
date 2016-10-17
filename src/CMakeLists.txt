add_library(FileUtils FileUtils.cpp ../test/TestTableManager.cpp ../test/TestTableManager.h)

add_library(Pager Pager.cpp ../test/TestTableManager.cpp ../test/TestTableManager.h)
target_link_libraries(Pager FileUtils)
add_library(Page Page.cpp ../test/TestTableManager.cpp ../test/TestTableManager.h)
target_link_libraries(Page FileUtils)

add_subdirectory(RecordManage)