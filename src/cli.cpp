#include <Parser/Lexer.h>
#include <Parser/ParserVal.h>
#include <Parser/AST/Nodes.h>
#include <Parser/Parser.tab.hpp>
#include <Parser/ParsingError.h>
#include <boost/assert.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <RecordManage/TableManager.h>
#include <cstdlib>
#include <stdlib.h>

using namespace std;
using namespace tinydbpp;
using namespace boost;

enum Mode {
  CLI,
  EvalFile
};
int main(int argc, char **argv) {
  int mode = CLI;
  const char *sqlFilePath;
  if (argc == 2) {
    mode = EvalFile;
    sqlFilePath = argv[1];
  }
  else if (argc == 1) {
    mode = CLI;
  }
  else {
    fprintf(stderr, "Wrong parameter");
    _exit(127);
  }

  log::core::get()->set_filter(log::trivial::severity >= log::trivial::fatal);

  if (mode == EvalFile) {
    ifstream ssin(sqlFilePath);
    stringstream ssout;
    Lexer lexer(ssin, ssout);
    std::shared_ptr<ast::Node> node;
    Parser parser(lexer, node);
    BOOST_ASSERT(parser.parse() == 0);
    BOOST_ASSERT(dynamic_cast<ast::Statements *>(node.get()));
    auto stmts = dynamic_pointer_cast<ast::Statements>(node);
    auto ss = stmts->get();
    for (auto &s: ss) {
      cout <<  s->exec() << endl;
    }
  }
  else {
    while (true) {
      string line;
      cout << "tinydb> ";
      std::getline(cin, line);
      if (line.size() == 0)
        break;
      stringstream ssin(line);
      stringstream ssout;
      Lexer lexer(ssin, ssout);
      std::shared_ptr<ast::Node> node;
      Parser parser(lexer, node);
      BOOST_ASSERT(parser.parse() == 0);
      BOOST_ASSERT(dynamic_cast<ast::Statements *>(node.get()));
      auto stmts = dynamic_pointer_cast<ast::Statements>(node);
      auto ss = stmts->get();
      for (auto &s: ss) {
        cout <<  s->exec() << endl;
      }
    }
    cout << "Good byte!" << endl;
  }
}
