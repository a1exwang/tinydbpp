#pragma once
#include <exception>
#include "ParserVal.h"
namespace tinydbpp {

class ParsingError :public std::runtime_error {
public:
  ParsingError(const std::string &str) :runtime_error(str) { }
  virtual std::string toString() const;
};

class TokenError :public ParsingError {
public:
  TokenError(const std::string &str, const ParserVal &val, const std::string &rawText)
          :ParsingError(str), parserVal(val), rawText(rawText) { }
  virtual std::string toString() const override;
private:
  ParserVal parserVal;
  std::string rawText;
};

class TypeError : public std::runtime_error {
public:
    std::string type1, type2;
    TypeError(const std::string & str, const std::string & t1, const std::string & t2): runtime_error(str), type1(t1), type2(t2){
    }
    virtual std::string toString() const {
        return std::string("Type ") + type1 + " and " + type2 + " do not incompatible";

    }
};
class DoubleFault :public std::runtime_error {
public:
  DoubleFault(const std::string &str) :std::runtime_error(str) {
  }
};
}
