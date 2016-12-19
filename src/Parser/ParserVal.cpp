#include "ParserVal.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>
#include "ParsingError.h"
#include <Parser/AST/Nodes.h>

using namespace std;
using namespace tinydbpp;

std::string ParserVal::toString() const {
  stringstream ss;
  stringstream ss1;
  string typeStr;

  switch (type) {
  case Int:
    typeStr = "Int";
    ss1 << iVal;
    break;
  case Double:
    typeStr = "Double";
    ss1 << dVal;
    break;
  case String:
    typeStr = "String";
    ss1 << strVal;
    break;
  default:
    typeStr = "Unknown";
    ss1 << string("int \"") << iVal << "\", double \"" << dVal << "\", str \"" << strVal << "\"";
    break;
  }
  ss << string("ParserVal<type: ") << type << ", " << typeStr << ">. " << ss1.str();

  return ss.str();
}

void ParserVal::becomeInt(const char *str, size_t len) {
  BOOST_ASSERT(len >= 0);
  this->type = Int;
  stringstream ss;
  ss << std::string(str, (size_t)len);
  ss >> iVal;
  if (ss.fail()) {
    throw TokenError("Integer overflow", *this, string(str, len));
  }
}

void ParserVal::becomeHexInt(const char *str, size_t len) {
  BOOST_ASSERT_MSG(len >= 3, "Lexer. Hex integer format error");
  this->type = Int;
  stringstream ss;
  ss << std::hex << std::string(str+2, len-2);
  ss >> iVal;
  if (ss.fail()) {
    throw TokenError("Hex integer overflow", *this, string(str, len));
  }
}

void ParserVal::becomeKeyword(const char *str, size_t len) {
  type = Keyword;
  strVal = std::string(str, len);
  /**
   * Lower case the string.
   * NOTE: use ::tolower rather than std::lower to avoid function overloading ambiguity
   */
  std::transform(strVal.begin(), strVal.end(), strVal.begin(), ::tolower);
}

void ParserVal::becomeKeyword(const char *str) {
  becomeKeyword(str, strlen(str));
}

void ParserVal::becomeIdentifier(const char *str, size_t len) {
  type = Identifier;
  strVal = string(str, len);
}

void ParserVal::makeCreateDbNode(const ParserVal &target) {
  node = std::make_shared<ast::SysManagement>(target, ast::Statement::Type::CreateDb);
}

void ParserVal::makeShowDbsNode() {
  node = std::make_shared<ast::SysManagement>(ast::Statement::Type::ShowDbs);
}

void ParserVal::makeShowTablesNode() {
  node = std::make_shared<ast::SysManagement>(ast::Statement::Type::ShowTables);
}

void ParserVal::makeDropDbNode(const ParserVal &target) {
  node = std::make_shared<ast::SysManagement>(target, ast::Statement::Type::DropDb);
}

void ParserVal::makeUseDbNode(const ParserVal &target) {
  node = std::make_shared<ast::SysManagement>(target, ast::Statement::Type::UseDb);
}

void ParserVal::becomeBasicSymbol(const char *str, size_t len) {
  type = BasicSymbol;
  strVal = string(str, len);
}

void ParserVal::makecreateTbNode(const ParserVal &target) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::CreateTable);
}

void ParserVal::makeDropTbNode(const ParserVal &target) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DropTable);
}

void ParserVal::makeDescribeTbNode(const ParserVal &) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DesribeTable);
}

void ParserVal::makeInsertTbNode(const ParserVal &, const ParserVal &) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::InsertItem);

}

void ParserVal::makeDeleteTbNode(const ParserVal &, const ParserVal &) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DeleteItem);

}

void ParserVal::makeUpdateTbNode(const ParserVal &, const ParserVal &) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::UpdateItem);
}

void ParserVal::makeSelectTbNode(const ParserVal &, const ParserVal &, const ParserVal &) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::SelectItem);

}

void ParserVal::makeCreateIdxNode(const ParserVal &) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::CreateIdx);

}

void ParserVal::makeDropIdxNode(const ParserVal &) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DropIdx);
}

