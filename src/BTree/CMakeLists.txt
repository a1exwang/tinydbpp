add_library(BTree BTree.cpp)
target_link_libraries(BTree RecordManager TableManager Pager Page FileUtils)
