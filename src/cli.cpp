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
#include <sys/time.h>

using namespace std;
using namespace tinydbpp;
using namespace boost;

enum Mode {
  CLI,
  EvalFile
};

int64_t timeToMicroSec(const timeval &time) {
  return time.tv_sec * 1000000 + time.tv_usec;
}


int eval(istream &ssin) {
  stringstream ssout;
  Lexer lexer(ssin, ssout);
  std::shared_ptr<ast::Node> node;
  Parser parser(lexer, node);
  try {
    if (parser.parse() != 0) {
      cerr << "Syntax error" << endl;
      return 0;
    }
    if (dynamic_cast<ast::Statements *>(node.get()) == nullptr) {
      cerr << "Syntax error, please check your SQL statement." << endl;
      return 0;
    }
    auto stmts = dynamic_pointer_cast<ast::Statements>(node);
    auto ss = stmts->get();

    struct timeval time;
    gettimeofday(&time, NULL); // Start Time
    int64_t totalStartTimeMs = timeToMicroSec(time);

    int count = 0;
    for (auto &s: ss) {
      gettimeofday(&time, NULL); // Start Time
      int64_t stmtStartTimeMs = timeToMicroSec(time);
      cout << s->exec() << endl;
      gettimeofday(&time, NULL); // Start Time
      int64_t deltaT = timeToMicroSec(time) - stmtStartTimeMs;
      cout << "Time: " << deltaT / 1000 << "ms" << endl;
      count++;
    }
    gettimeofday(&time, NULL);  //END-TIME
    double deltaT = timeToMicroSec(time) - totalStartTimeMs;
    cout << "Total time: " << deltaT / 1000 << "ms" << endl;
    return count;
  }
  catch (std::runtime_error re) {
    cerr << "Syntax error" << endl;
    return 0;
  }
}

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
    ifstream is(sqlFilePath);
    eval(is);
  }
  else {
    while (true) {
      string line;
      cout << "tinydb> ";
      std::getline(cin, line);
      if (line.size() == 0)
        break;
      stringstream is(line);
      int count = eval(is);
      cout << endl;
    }

    cout << "Good Bye!" << endl;
  }
}
