#include <Parser/Lexer.h>
#include <Parser/ParserVal.h>
#include <Parser/AST/Nodes.h>
#include <Parser/Parser.tab.hpp>
#include <Parser/ParsingError.h>
#include <boost/assert.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <RecordManage/TableManager.h>
#include <cstdlib>
#include <memory>
using namespace std;
using namespace tinydbpp;
using namespace boost;

int main(int argc, char **argv) {
  stringstream ssin, ssout;
  Lexer lexer(ssin, ssout);
  std::shared_ptr<ast::Node> node;
  Parser parser(lexer, node);
  ssin << "create database my_test;" << endl;
  ssin << "use database my_test;" << endl;
  ssin << "create table T1 ( id int(10), p varchar(10), pp int(10) not null,  PRIMARY KEY  (id));"<<endl;
  ssin << "create table T2 ( id int(10), p varchar(10), pp int(10) not null,  PRIMARY KEY  (id));"<<endl;
  ssin << "create table T3 ( id int(10), p3 varchar(10), pp3 int(10) not null,  PRIMARY KEY  (id));"<<endl;
  ssin << "insert into T1 values (1, 't1_varchar1', 10),(2, 't1_varchar2', 20);"<<endl;
  ssin << "insert into T2 values (1, 't2_varchar1', 10),(2, 't2_varchar2', 20);"<<endl;
  ssin << "insert into T3 values (1, 't3_varchar1', 10),(2, 't3_varchar2', 20), (3, 't3_varchar3', 30);"<<endl;
  ssin << "update T1 set id = 123 where id = 1;";
  ssin << "select * from T1 where id = 123;" << endl;
  BOOST_ASSERT(parser.parse() == 0);
  auto stmts = dynamic_pointer_cast<ast::Statements>(node);
  auto ss = stmts->get();
  for (auto &s: ss) {
    cout << "result: " << s->exec() << endl;
  }
}