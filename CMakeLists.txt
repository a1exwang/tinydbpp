cmake_minimum_required(VERSION 2.8)
project(tinydbpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(src)
link_libraries(boost_system boost_log boost_thread pthread)
add_definitions(-DBOOST_LOG_DYN_LINK)

add_subdirectory(src)
add_subdirectory(test)
