#include <Parser/Lexer.h>
#include <Parser/ParserVal.h>
#include <Parser/AST/Nodes.h>
#include <Parser/Parser.tab.hpp>
#include <Parser/ParsingError.h>
#include <boost/assert.hpp>
#include <iostream>
#include <sstream>

/**
 * Make sure to define these two macros before including `boost/test/unit_test.hpp`
 */
// This macro tells boost to link her own `main` function.
#define BOOST_TEST_DYN_LINK
// This macro describe the test module name
#define BOOST_TEST_MODULE Parser
#include <boost/test/unit_test.hpp>

using namespace tinydbpp;
using namespace std;

BOOST_AUTO_TEST_CASE(emptyStatement) {
  stringstream ssin, ssout;
  Lexer lexer(&ssin, &ssout);
  shared_ptr<ast::Node> node;
  Parser parser(lexer, node);

  ssin << "";

  BOOST_REQUIRE(parser.parse() == 0);
  BOOST_REQUIRE(dynamic_cast<ast::Statements*>(node.get()));
  auto stmts = dynamic_pointer_cast<ast::Statements>(node);
  auto ss = stmts->get();

  BOOST_REQUIRE(ss.size() == 0);
}

BOOST_AUTO_TEST_CASE(showDatabases) {
  stringstream ssin, ssout;
  Lexer lexer(&ssin, &ssout);
  shared_ptr<ast::Node> node;
  Parser parser(lexer, node);

  ssin << "show databases;" << endl;
  ssin << "create database test_database;" << endl;
  ssin << "drop database test_database;" << endl;
  ssin << "use database test_database;" << endl;
  ssin << "show tables;" << endl;

  BOOST_REQUIRE(parser.parse() == 0);
  BOOST_REQUIRE(dynamic_cast<ast::Statements*>(node.get()));
  auto stmts = dynamic_pointer_cast<ast::Statements>(node);
  auto ss = stmts->get();
  BOOST_REQUIRE(ss[0]->getType() == ast::Statement::Type::ShowDbs);
  BOOST_REQUIRE(ss[1]->getType() == ast::Statement::Type::CreateDb);
  BOOST_REQUIRE(dynamic_pointer_cast<ast::SysManagement>(ss[1])->getTarget()->getStrVal() == "test_database");
  BOOST_REQUIRE(ss[2]->getType() == ast::Statement::Type::DropDb);
  BOOST_REQUIRE(dynamic_pointer_cast<ast::SysManagement>(ss[2])->getTarget()->getStrVal() == "test_database");
  BOOST_REQUIRE(ss[3]->getType() == ast::Statement::Type::UseDb);
  BOOST_REQUIRE(dynamic_pointer_cast<ast::SysManagement>(ss[3])->getTarget()->getStrVal() == "test_database");
  BOOST_REQUIRE(ss[4]->getType() == ast::Statement::Type::ShowTables);
}
