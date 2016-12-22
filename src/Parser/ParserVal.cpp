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

void ParserVal::makeCreateDbNode(const ParserVal & ch0) {
  node = std::make_shared<ast::SysManagement>(ch0, ast::Statement::Type::CreateDb);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
}

void ParserVal::makeShowDbsNode() {
  node = std::make_shared<ast::SysManagement>(ast::Statement::Type::ShowDbs);
}

void ParserVal::makeShowTablesNode() {
  node = std::make_shared<ast::SysManagement>(ast::Statement::Type::ShowTables);
}

void ParserVal::makeDropDbNode(const ParserVal &ch0) {
  node = std::make_shared<ast::SysManagement>(ch0, ast::Statement::Type::DropDb);
    node->ch[0] = std::make_shared<ParserVal>(ch0);

}

void ParserVal::makeUseDbNode(const ParserVal &ch0) {
  node = std::make_shared<ast::SysManagement>(ch0, ast::Statement::Type::UseDb);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
}

void ParserVal::becomeBasicSymbol(const char *str, size_t len) {
  type = BasicSymbol;
  strVal = string(str, len);
}

void ParserVal::makecreateTbNode(const ParserVal &ch0, const ParserVal &ch1) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::CreateTable);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
    node->ch[1] = std::make_shared<ParserVal>(ch1);
}

void ParserVal::makeDropTbNode(const ParserVal & ch0) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DropTable);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
}

void ParserVal::makeDescribeTbNode(const ParserVal & ch0) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DesribeTable);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
}

void ParserVal::makeInsertTbNode(const ParserVal & ch0, const ParserVal & ch1) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::InsertItem);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
    node->ch[1] = std::make_shared<ParserVal>(ch1);
}

void ParserVal::makeDeleteTbNode(const ParserVal & ch0, const ParserVal & ch1) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DeleteItem);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
    node->ch[1] = std::make_shared<ParserVal>(ch1);

}

void ParserVal::makeUpdateTbNode(const ParserVal & ch0, const ParserVal & ch1, const ParserVal & ch2) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::UpdateItem);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
    node->ch[1] = std::make_shared<ParserVal>(ch1);
    node->ch[2] = std::make_shared<ParserVal>(ch2);
}

void ParserVal::makeSelectTbNode(const ParserVal & ch0, const ParserVal & ch1, const ParserVal & ch2) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::SelectItem);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
    node->ch[1] = std::make_shared<ParserVal>(ch1);
    node->ch[2] = std::make_shared<ParserVal>(ch2);

}

void ParserVal::makeCreateIdxNode(const ParserVal & ch0) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::CreateIdx);
    node->ch[0] = std::make_shared<ParserVal>(ch0);

}

void ParserVal::makeDropIdxNode(const ParserVal & ch0) {
    node = std::make_shared<ast::TableManagement>(ast::Statement::Type::DropIdx);
    node->ch[0] = std::make_shared<ParserVal>(ch0);
}

