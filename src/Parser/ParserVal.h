#pragma once
#include <string>
#include <exception>
#include <stdexcept>
#include <memory>

namespace tinydbpp {
namespace ast {
class Node;
class SysManagement;
}
class ParserVal {
public:
  enum Type {
    Unknown = -1,
    Int, String, Double, Keyword, Identifier, BasicSymbol
  };
public:
  typedef std::shared_ptr<ast::Node> SPNode;
  explicit ParserVal(int iVal, SPNode node = nullptr) : type(Int), iVal(iVal), node(node) { }
  explicit ParserVal(const std::string &strVal, SPNode node = nullptr) : type(String), strVal(strVal), node(node) { }
  ParserVal(SPNode node = nullptr) :type(Unknown), node(node) {}

  Type getType() const { return type; }
  int getIntVal() const { return iVal; }
  std::string getStrVal() const { return strVal; }
  double getDoubleVal() const { return dVal; }

  void becomeStringLiteral(const char *str, size_t len) {
    type = String;
    auto lit = std::string(str, len);
    /**
     * Delete quotes
     */
    strVal = lit.substr(1, lit.length() - 2);
  }
  void becomeKeyword(const char *str);
  void becomeKeyword(const char *str, size_t len);
  void becomeIdentifier(const char *str, size_t len);
  void becomeInt(const char *str, size_t len);
  void becomeHexInt(const char *str, size_t len);
  void becomeBasicSymbol(const char *str, size_t len);

  SPNode getNode() { return node; }
  void makeShowDbsNode();
  void makeCreateDbNode(const ParserVal &target);
  void makeDropDbNode(const ParserVal &target);
  void makeUseDbNode(const ParserVal &target);
  void makeShowTablesNode();
  void makecreateTbNode(const ParserVal &target);
  void makeDropTbNode(const ParserVal &target);
    void makeDescribeTbNode(const ParserVal &);
    void makeInsertTbNode(const ParserVal &, const ParserVal &);
    void makeDeleteTbNode(const ParserVal &,const ParserVal &);
    void makeUpdateTbNode(const ParserVal &, const ParserVal &);
    void makeSelectTbNode(const ParserVal &, const ParserVal &, const ParserVal &);
    void makeCreateIdxNode(const ParserVal &);
    void makeDropIdxNode(const ParserVal &);
  std::string toString() const;

  Type type = Unknown;
  int iVal = 0;
  double dVal = 0;
  std::string strVal = "";
  SPNode node;
};

}