add_library(Pager Pager.cpp PagerPerfMon.cpp)
target_link_libraries(Pager FileUtils)
add_library(Page Page.cpp)
target_link_libraries(Page FileUtils)
