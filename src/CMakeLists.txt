add_library(FileUtils FileUtils.cpp )

add_subdirectory(Pager)
add_subdirectory(RecordManage)
add_subdirectory(Parser)

add_executable(cli cli.cpp)
target_link_libraries(cli AST SQLParser TestUtils)
