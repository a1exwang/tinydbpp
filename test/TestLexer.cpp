#include <Parser/Lexer.h>
#include <boost/assert.hpp>
#include <iostream>
#include <sstream>

/**
 * Make sure to define these two macros before including `boost/test/unit_test.hpp`
 */
// This macro tells boost to link her own `main` function.
#define BOOST_TEST_DYN_LINK
// This macro describe the test module name
#define BOOST_TEST_MODULE Lexer
#include <boost/test/unit_test.hpp>
#include <Parser/ParsingError.h>

using namespace std;
using namespace tinydbpp;

BOOST_AUTO_TEST_CASE(integer) {
  stringstream ssin;
  Lexer lexer(&ssin, &cout);
  ssin << "1234";
  ParserVal iVal;
  BOOST_REQUIRE(lexer.lex(&iVal) != 0);
  BOOST_REQUIRE(iVal.getType() == ParserVal::Type::Int);
  BOOST_REQUIRE(iVal.getIntVal() == 1234);
}
BOOST_AUTO_TEST_CASE(integerHex) {
  stringstream ssin;
  Lexer lexer(&ssin, &cout);
  ssin << "0x1234";
  ParserVal iVal;
  BOOST_REQUIRE(lexer.lex(&iVal) != 0);
  BOOST_REQUIRE(iVal.getType() == ParserVal::Type::Int);
  BOOST_REQUIRE(iVal.getIntVal() == 0x1234);
}

BOOST_AUTO_TEST_CASE(integerOverflow) {
  stringstream ssin;
  Lexer lexer(&ssin, &cout);
  ssin << "1234567890123";
  ParserVal iVal;

  BOOST_REQUIRE_THROW(lexer.lex(&iVal), TokenError);
}

/**
 * All keyword token has type `Keyword`, and downcased string saved in strVal.
 */
BOOST_AUTO_TEST_CASE(keyword) {
  stringstream ssin;
  Lexer lexer(&ssin, &cout);
  ssin << "create TABLE";
  ParserVal token;

  BOOST_REQUIRE(lexer.lex(&token) == Lexer::Token::KW_CREATE);
  BOOST_REQUIRE(token.getType() == ParserVal::Type::Keyword);
  BOOST_REQUIRE(token.getStrVal() == string("create"));

  BOOST_REQUIRE(lexer.lex(&token) == Lexer::Token::KW_TABLE);
  BOOST_REQUIRE(token.getType() == ParserVal::Type::Keyword);
  BOOST_REQUIRE(token.getStrVal() == string("table"));
}

/**
 * String literal
 */
BOOST_AUTO_TEST_CASE(stringLiteral) {
  stringstream ssin;
  Lexer lexer(&ssin, &cout);
  ssin << "'string literal'";
  ParserVal token;

  BOOST_REQUIRE(lexer.lex(&token) == Lexer::Token::STRING);
  BOOST_REQUIRE(token.getType() == ParserVal::Type::String);
  BOOST_REQUIRE(token.getStrVal() == string("string literal"));
}
