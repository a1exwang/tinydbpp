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

};
}
