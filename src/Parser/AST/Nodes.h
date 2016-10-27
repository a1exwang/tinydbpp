#pragma once
#include <vector>
#include <memory>
#include <Parser/ParserVal.h>

namespace tinydbpp {
namespace ast {

class Node {
public:
  virtual ~Node() {}
};

class Statement :public Node {
public:
  enum Type {

    SysManagementBegins,
    ShowDbs,
    CreateDb,
    DropDb,
    UseDb,
    ShowTables,
    SysManagementEnds,

    TableManagementBegins,
    TableManagementEnds,
  };
public:
  Statement(Type type) :type(type) { }
  virtual ~Statement() { }
  Type getType() const { return type; }
private:
  Type type;
};
class SysManagement :public Statement {
public:
  SysManagement(Statement::Type type) :Statement(type) { }
  SysManagement(const ParserVal& val, Statement::Type type)
    :Statement(type), target(std::make_shared<ParserVal>(val)) { }
  virtual ~SysManagement() { }
  std::shared_ptr<const ParserVal> getTarget() const;
private:
  std::shared_ptr<ParserVal> target;
};

class Statements :public Node {
public:
  Statements() { }
  virtual ~Statements() { }
  const std::vector<std::shared_ptr<Statement>> get() const {
    return statements;
  }
  void addStatementToFront(std::shared_ptr<Statement> stmt);

private:
  std::vector<std::shared_ptr<Statement>> statements;
};

}
}
