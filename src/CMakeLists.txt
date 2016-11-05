add_library(FileUtils FileUtils.cpp )

add_library(Pager Pager.cpp )
target_link_libraries(Pager FileUtils PagerPerfMon)
add_library(Page Page.cpp)
target_link_libraries(Page FileUtils TestUtils)
add_library(PagerPerfMon PagerPerfMon.cpp)
target_link_libraries(PagerPerfMon FileUtils)

add_subdirectory(RecordManage)
