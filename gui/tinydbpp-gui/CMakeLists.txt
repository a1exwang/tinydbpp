find_package(Qt5Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(mainwindow mainwindow.cpp)
target_link_libraries (mainwindow Qt5::Widgets AST SQLParser)

add_executable(tinydbpp-qt main.cpp)
target_link_libraries (tinydbpp-qt mainwindow)
