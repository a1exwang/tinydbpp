#pragma once
#include <vector>
#include <memory>
#include <Parser/ParserVal.h>
#include <functional>
#include <json.hpp>
#include <boost/assert.hpp>
#include <RecordManage/TableManager.h>
#include <string>

namespace tinydbpp {
namespace ast {
using json = nlohmann::json;
using std::string;
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
    Value(std::string _t):type(_t){}
    Value(std::string _t, std::string colv):type(_t){
        if(*(colv.end() - 1) == 1) type = "NULL";
        else if(_t == "int") iVal = *(int*)colv.c_str();
        else if(_t == "varchar") strVal = std::string(colv.begin(), colv.end() - 1);
    }
    std::string toString(std::string req_type)
    {
        BOOST_ASSERT(type == "NULL" || req_type == type);
        if(type == "NULL"){
            if(req_type == "varchar") return std::string(1, (char)1);
            else if(req_type == "varchar") return std::string(4, '\0') + std::string(1, (char)1);
            else BOOST_ASSERT(0);
        }else if(type == "int"){
            std::string v_str(5, '\0');
            v_str.replace(v_str.begin(), v_str.begin() + 4, std::string((char*)&(iVal), (char*)&(iVal)+ 4));
            return v_str;
        }else if(type == "varchar"){
            return strVal + std::string(1, '\0');
        }else BOOST_ASSERT(0);
        return "";
    }
    //col int string
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

typedef std::function< std::vector<Value> (const Item &, const std::string&) > Selector;

class WhereClause : public Node{
public:
    std::vector<std::string> names, ops;
    std::vector<Value> exprs;
    void becomeCompare(const std::string& colname, const std::string& op, Value v);
    void becomeIsNull(const std::string& colname);
    void becomeIsNotNull(const std::string& colname);
    void becomeAnd(std::shared_ptr<WhereClause> w1, std::shared_ptr<WhereClause> w2);
    void becomeLike(const std::string &colname, const std::string &regex);
    /*
     * @param actually two return value, whether can be optimized by index and which col
     * @return which table to reduce
     * find the best table to reduce
     */
    std::string getNextAssignTableName(bool & can_index, int & col_index, std::string& , const std::vector<std::string>& tables);
    /*
     * @param table_name
     * @return checker function
     * get a checker function to check a item in table "table_name" whether meet the condition
     * must guarantee that if column has no prefix "table_", it belongs to table "table_name"
     */
    Checker getChecker(std::string table_name);
    void dfs(std::vector< std::vector<Value> > &ans, const std::vector<std::string>& table_names, const std::vector<Value>& prefix,
             const Selector & selector, std::vector<std::string>& assigned_table);
    WhereClause assign(const std::string& table_name, const Item & item);
};

class SetClause : public Node{
public:
    std::vector<std::string> cols;
    std::vector<Value> values;
    void push_back(std::string colname, Value v){
        cols.push_back(colname);
        values.push_back(v);
    }
    Changer getChanger(const std::string& table_name) {
        auto td = TableManager::getInstance()->getTableDescription(table_name);
        std::vector<int> offsets;
        std::vector<std::string> embed_values;
        for(int i = 0;i < cols.size();i++)
        {
            int offset = td->getOffset(cols[i]);
            offsets.push_back(offset);
            embed_values.push_back(values[i].toString(td->col_type[offset]));
        }
        return ([offsets, embed_values](Item & item){
            for(int i = 0;i < offsets.size();i++)
                item[offsets[i]] = embed_values[i];
            return;
        });
    }
};

class ColList : public  Node{
public:
    std::vector<std::string> cols;
    void push_back(std::string colname){ cols.push_back(colname);}
    static void split(const std::string& full_name, const std::string& default_table, std::string & table, std::string & col){
        size_t pos = full_name.find(".");
        table =  pos == std::string::npos? default_table : string(full_name.begin(), full_name.begin() + pos);
        col = pos == std::string::npos? full_name : string(full_name.begin() + pos + 1, full_name.end());
    }
};

class SelectCols : public Node{
public:
    bool isAll;
    std::shared_ptr<ColList> col_list;
    SelectCols():isAll(false),col_list(nullptr){}
    void setAll(){ isAll = true;}
    void setColList(std::shared_ptr<ColList> _c){ col_list = _c;}
    Selector getSelector(const std::vector<std::string>& table_names) {
        std::vector< std::vector<int> > offsets;
        for(auto & table_name : table_names)
        {
            std::vector<int> this_offsets;
            auto td = TableManager::getInstance()->getTableDescription(table_name);
            if(isAll)
                for(int i = 0;i < td->col_name.size();i++)
                    this_offsets.push_back(i);
            else
                for(std::string & full_name : col_list->cols){
                    std::string this_table, this_col;
                    ColList::split(full_name, table_name, this_table, this_col);
                    if(this_table == table_name)
                        this_offsets.push_back(td->getOffset(this_col));
                }
            offsets.push_back(this_offsets);
        }
        return [table_names, offsets](const Item & item, const std::string& table_name)-> std::vector<Value>{
            std::vector<Value> ret;
            auto td = TableManager::getInstance()->getTableDescription(table_name);
            for(int i = 0;i < table_names.size();i++)
                if(table_name == table_names[i])
                    for(int j = 0;j < offsets[i].size();j++)
                        ret.push_back(Value(td->col_type[j], item[j]));
            return ret;
        };
    }
};
class TableList : public  Node{
    public:
        std::vector<std::string> tables;
        void push_back(std::string tablename){ tables.push_back(tablename);}
};
}
}
