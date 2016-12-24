#include <Parser/Lexer.h>
#include <Parser/ParserVal.h>
#include <Parser/AST/Nodes.h>
#include <Parser/Parser.tab.hpp>
#include <Parser/ParsingError.h>
#include <boost/assert.hpp>
#include <iostream>
#include <sstream>
#include <RecordManage/TableManager.h>

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
  Lexer lexer(ssin, ssout);
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
  Lexer lexer(ssin, ssout);
  shared_ptr<ast::Node> node;
  Parser parser(lexer, node);
    ssin << "create database test_database;" << endl;
  ssin << "show databases;" << endl;
    ssin << "use database test_database;" << endl;
    ssin << "drop database test_database;" << endl;
    ssin << "show databases;" << endl;
  ssin << "show tables;" << endl;

  BOOST_REQUIRE(parser.parse() == 0);
  auto stmts = dynamic_pointer_cast<ast::Statements>(node);
  auto ss = stmts->get();
    for(auto & s: ss){
        cout << s->exec()<<endl;
    }
//  BOOST_REQUIRE(ss[0]->getType() == ast::Statement::Type::ShowDbs);
//  BOOST_REQUIRE(ss[1]->getType() == ast::Statement::Type::CreateDb);
//  BOOST_REQUIRE(dynamic_pointer_cast<ast::SysManagement>(ss[1])->getTarget()->getStrVal() == "test_database");
//  BOOST_REQUIRE(ss[2]->getType() == ast::Statement::Type::DropDb);
//  BOOST_REQUIRE(dynamic_pointer_cast<ast::SysManagement>(ss[2])->getTarget()->getStrVal() == "test_database");
//  BOOST_REQUIRE(ss[3]->getType() == ast::Statement::Type::UseDb);
//  BOOST_REQUIRE(dynamic_pointer_cast<ast::SysManagement>(ss[3])->getTarget()->getStrVal() == "test_database");
//  BOOST_REQUIRE(ss[4]->getType() == ast::Statement::Type::ShowTables);
}
BOOST_AUTO_TEST_CASE(createTable) {
    stringstream ssin, ssout;
    Lexer lexer(ssin, ssout);
    shared_ptr<ast::Node> node;
    Parser parser(lexer, node);
    ssin << "create database test_database;" << endl;
    ssin << "use database test_database;" << endl;
    ssin << "create table T1 ( id int(10), p varchar(10), pp int(10) not null,  PRIMARY KEY  (id));"<<endl;
    ssin << "create table T2 ( id int(10), p varchar(10), pp int(10) not null,  PRIMARY KEY  (id));"<<endl;
    ssin << "create table T3 ( id int(10), p3 varchar(10), pp3 int(10) not null,  PRIMARY KEY  (id));"<<endl;
    ssin << "desc T1;"<<endl;
    ssin << "insert into T1 values (1, '123a', 1),(2, 'fffffffffffffffffff', 222);"<<endl;
    ssin << "insert into T2 values (1, '123b', 1123),(3, 'bbb', 234);"<<endl;
    ssin << "insert into T3 values (1, '123b', 1123),(4, 'bbb', 234), (2, 'bbb', 234);"<<endl;
    ssin << "select * from T1, T2, T3 where T3.id > 1;" << endl;
    BOOST_REQUIRE(parser.parse() == 0);
    BOOST_REQUIRE(dynamic_cast<ast::Statements *>(node.get()));
    auto stmts = dynamic_pointer_cast<ast::Statements>(node);
    auto ss = stmts->get();
    for (auto &s: ss) {
        cout << "result: " << s->exec() << endl;
    }
    auto td = TableManager::getInstance()->getTableDescription("T1");
    BOOST_REQUIRE(td != nullptr);
    BOOST_REQUIRE(td->pattern.size() == 3);
}

