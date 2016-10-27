#pragma once

#include <Parser/ParserVal.h>
#include <Parser/Parser.tab.hpp>
#ifndef __FLEX_LEXER_H
#define yyFlexLexer SQLFlexLexer
#include "FlexLexer.h"
#undef yyFlexLexer
#endif

namespace tinydbpp {

class Lexer :public SQLFlexLexer {
public:
  typedef tinydbpp::Parser::token::yytokentype Token;
public:
  Lexer(std::istream &in, std::ostream &out) :SQLFlexLexer(in, out) {
  }
  int yylex();
  int lex(ParserVal *v) {
    int ret = yylex();
    *v = yylval;
    return ret;
  }
private:
  ParserVal yylval;
};

}
