#include "ParsingError.h"
#include <sstream>

using namespace std;
using namespace tinydbpp;

std::string ParsingError::toString() const {
  stringstream ss;
  ss << "ParsingError. message: \"" << this->what() << "\"";
  return ss.str();
}

std::string TokenError::toString() const {
  stringstream ss;
  ss << "TokenError. ParserVal: \"" << this->parserVal.toString() << "\"" << endl;
  ss << "\tMessage: \"" << this->what() << "\"";
  return ss.str();
}
