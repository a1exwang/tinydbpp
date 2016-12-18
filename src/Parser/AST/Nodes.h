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


class Field : public Node{
public:
    virtual ~Field(){}
    Field(){
        can_null = true;
        is_key = false;
        is_primary_key_stmt = false;
        size = 0;
    }
    Field(std::string _n, std::string _t, int s, bool _null, bool _key, bool _is_p_stmt = false):name(_n),type(_t), can_null(_null), is_key(_key)
            ,is_primary_key_stmt(_is_p_stmt){
    }
    std::string name;
    std::string type;
    int size;
    bool can_null;
    bool is_key;
    bool is_primary_key_stmt;
};

class FieldList : public Node{
public:
    std::vector<Field> vec;
    void checkPrimaryKey(){
        for(auto &f : vec)
            if(f.is_primary_key_stmt)
                for(auto &t : vec)
                    if(t.name == f.name && !t.is_primary_key_stmt)
                        t.is_key = true;
    }
};




}
}
