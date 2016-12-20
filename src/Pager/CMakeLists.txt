include_directories(.)
add_library(Pager Pager.cpp)
target_link_libraries(Pager FileUtils PagerPerfMon)
add_library(Page Page.cpp)
target_link_libraries(Page FileUtils)

add_library(PagerPerfMon PagerPerfMon.cpp)
