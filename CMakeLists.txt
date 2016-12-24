cmake_minimum_required(VERSION 2.8)
project(tinydbpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Optimization flags
#SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -D_DEBUG")
#SET(CMAKE_CXX_FLAGS_RELEASE "-O2")


include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/third_party/json/src)
# Link libraries
if(UNIX)
  if(APPLE)
    link_libraries(boost_system boost_log-mt boost_thread-mt boost_unit_test_framework-mt boost_filesystem-mt pthread)
  else() # Linux
    link_libraries(boost_system boost_log boost_thread boost_unit_test_framework boost_filesystem pthread)
  endif()
else()
  message(FATAL_ERROR, "Does not support non-unix platform")
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

# Enable all warnings
add_definitions(-Wall)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(gui/tinydbpp-gui)
