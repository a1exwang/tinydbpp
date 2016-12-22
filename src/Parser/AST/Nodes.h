#pragma once
#include <vector>
#include <memory>
#include <Parser/ParserVal.h>
#include <functional>
#include <json.hpp>
namespace tinydbpp {
namespace ast {
using json = nlohmann::json;

class Node {
public:
  std::shared_ptr<ParserVal> ch[3];
  Node():ch{nullptr}{}
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
      CreateTable,
      DropTable,
      DesribeTable,
      InsertItem,
      DeleteItem,
      UpdateItem,
      SelectItem,
    TableManagementEnds,

      CreateIdx,
      DropIdx
  };
public:
    virtual json exec();
  Statement(Type type);
  virtual ~Statement();
  Type getType() const { return type; }
private:
  Type type;
};
class SysManagement :public Statement {
public:
  SysManagement(Statement::Type type) :Statement(type) { }
  SysManagement(const ParserVal& val, Statement::Type type)
    :Statement(type), target(std::make_shared<ParserVal>(val)) { }
  virtual ~SysManagement();
  std::shared_ptr<const ParserVal> getTarget() const;
private:
  std::shared_ptr<ParserVal> target;
};

class TableManagement :public Statement {
public:
    TableManagement(Statement::Type type);
    virtual ~TableManagement();
private:
};
class Statements :public Node {
public:
  Statements() { }
  virtual ~Statements();
  const std::vector<std::shared_ptr<Statement>> get() const {
    return statements;
  }
  void addStatementToFront(std::shared_ptr<Statement> stmt);

private:
  std::vector<std::shared_ptr<Statement>> statements;
};


class Field : public Node{
public:
    virtual ~Field();
    Field(){
        can_null = true;
        is_key = false;
        is_primary_key_stmt = false;
        size = 0;
    }
    Field(std::string _n, std::string _t, int s, bool _null, bool _key, bool _is_p_stmt = false):name(_n),type(_t),size(s),can_null(_null), is_key(_key)
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

class Value : public Node{
public:
    std::string type;
    std::string strVal;
    int iVal;
    Value(std::string _t):strVal(_t){}
};
class ValueList : public Node{
public:
    std::vector<std::shared_ptr<Value>> vec;
    void push_back(std::shared_ptr<Value> p){
        vec.push_back(p);
    }
};
class ValueLists : public Node{
public:
    std::vector<std::shared_ptr<ValueList>> vec;
    void push_back(std::shared_ptr<ValueList> p){
        vec.push_back(p);
    }
};

class WhereClause : public Node{
public:
    std::vector<std::string> names, ops;
    std::vector<Value> exprs;
    void becomeCompare(const std::string& colname, const std::string& op, Value v);
    void becomeIsNull(const std::string& colname);
    void becomeIsNotNull(const std::string& colname);
    void becomeAnd(std::shared_ptr<WhereClause> w1, std::shared_ptr<WhereClause> w2);
    std::string getNextAssignTableName(); // used in select multiple tables
    WhereClause afterAssign();// use in generate new whereclause reducing one table
    std::function<bool(const std::vector<std::string>&)> getChecker();// get checker both select & update & delete
};

class SetClause : public Node{
public:
    std::vector<std::string> cols;
    std::vector<Value> values;
    void push_back(std::string colname, Value v){
        cols.push_back(colname);
        values.push_back(v);
    }
};

class ColList : public  Node{
public:
    std::vector<std::string> cols;
    void push_back(std::string colname){ cols.push_back(colname);}
};

class Selector : public Node{
public:
    bool isAll;
    std::shared_ptr<ColList> col_list;
    Selector():isAll(false),col_list(nullptr){}
    void setAll(){ isAll = true;}
    void setColList(std::shared_ptr<ColList> _c){ col_list = _c;}
};
class TableList : public  Node{
    public:
        std::vector<std::string> tables;
        void push_back(std::string tablename){ tables.push_back(tablename);}
};
}
}
