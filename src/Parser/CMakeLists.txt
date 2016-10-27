find_package(BISON 3)
find_package(FLEX 2)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

BISON_TARGET(Parser Parser.yy ${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.cpp)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

add_library(ParserVal ParserVal.cpp)
add_library(ParsingError ParsingError.cpp)
add_library(AST AST/Nodes.cpp)
add_library(SQLParser ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_link_libraries(SQLParser AST ParserVal ParsingError)

