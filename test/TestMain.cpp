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
using namespace std;
using namespace tinydbpp;
using namespace boost;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Wrong parameter");
    exit(127);
  }

  ifstream ssin(argv[1]);
  stringstream ssout;
  Lexer lexer(ssin, ssout);
  std::shared_ptr<ast::Node> node;
  Parser parser(lexer, node);
  BOOST_ASSERT(parser.parse() == 0);
  BOOST_ASSERT(dynamic_cast<ast::Statements *>(node.get()));
  auto stmts = dynamic_pointer_cast<ast::Statements>(node);
  auto ss = stmts->get();
  for (auto &s: ss) {
    cerr << "result: " << s->exec() << endl;
  }
}