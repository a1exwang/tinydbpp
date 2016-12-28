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
class NoThisColumnError : public std::runtime_error {
public:
    std::string table_name, col_name;
    NoThisColumnError(const std::string & t1, const std::string & t2): runtime_error("NoThisColumn"), table_name(t1), col_name(t2){
    }
    virtual std::string toString() const {
        return std::string("Table ") + table_name + " has not a column named " + col_name + "!";

    }
};
class DoubleFault :public std::runtime_error {
public:
  DoubleFault(const std::string &str) :std::runtime_error(str) {
  }
};
}
