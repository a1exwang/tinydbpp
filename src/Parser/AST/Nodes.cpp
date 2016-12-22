
#include "Nodes.h"
#include <Parser/ParserVal.h>
#include <cassert>
#include <iostream>
#include <FileUtils.h>
#include <Pager/Pager.h>
#include <Pager/Page.h>
#include <RecordManage/TableManager.h>
#include <stdlib.h>
#include <cmath>
#include <BTree/BTreePlus.h>
#include <config.h>
using namespace std;
using namespace tinydbpp::ast;

std::shared_ptr<const tinydbpp::ParserVal> SysManagement::getTarget() const {
  return target;
}

SysManagement::~SysManagement() { }

void Statements::addStatementToFront(std::shared_ptr<Statement> stmt) {
  statements.insert(statements.begin(), stmt);
}

Statements::~Statements() { }

void WhereClause::becomeCompare(const std::string &colname, const std::string &op, Value v) {
    names.push_back(colname);
    ops.push_back(op);
    exprs.push_back(v);
}

void WhereClause::becomeIsNull(const std::string &colname) {
    names.push_back(colname);
    ops.push_back("isnull");
    exprs.push_back(Value("null"));
}

void WhereClause::becomeIsNotNull(const std::string &colname) {
    names.push_back(colname);
    ops.push_back("isnotnull");
    exprs.push_back(Value("null"));
}

void WhereClause::becomeAnd(std::shared_ptr<WhereClause> w1, std::shared_ptr<WhereClause> w2) {
    names = w1->names;
    ops = w1->ops;
    exprs = w1->exprs;
    for(size_t i = 0;i < w2->names.size();i++){
        names.push_back(w2->names[i]);
        ops.push_back(w2->ops[i]);
        exprs.push_back(w2->ops[i]);
    }
}

std::string WhereClause::getNextAssignTableName() {


}

/*
 * func = [op, v](const std::vector<std::string>& item, const std::vector<int>& offset,const std::vector<std::string>& types){
        if(v.type == "col"){
            if(types[0] != types[1]) assert(0);// type check failed
            if(types[0] == "int"){
                int a = *(int *)item[offset[0]].c_str();
                int b = *(int *)item[offset[1]].c_str();
                char a_null = *(item[offset[0]].c_str() + 4);
                char b_null = *(item[offset[1]].c_str() + 4);
                if(a_null == 1 || b_null == 1) return false;
                if(a_null == 2 || b_null == 2) return true;
                if(op == "=") return a == b;
                    else if (op == "<") return a < b;
                    else if (op == ">") return a > b;
                    else if (op == "<>") return a != b;
                    else if (op == "<=") return a <= b;
                    else if (op == ">=") return a >= b;
                else std::cerr<< "error op: " << op <<std::endl;
            }else if(types[0] == "string"){
                std::string a(item[offset[0]].begin(), item[offset[0]].end() - 1);
                std::string b(item[offset[1]].begin(), item[offset[1]].end() - 1);
                char a_null = *(item[offset[0]].end() - 1);
                char b_null = *(item[offset[1]].end() - 1);
                if(a_null == 1 || b_null == 1) return false;
                if(a_null == 2 || b_null == 2) return true;
                if(op == "=") return a == b;
                    else if (op == "<") return a < b;
                    else if (op == ">") return a > b;
                    else if (op == "<>") return a != b;
                    else if (op == "<=") return a <= b;
                    else if (op == ">=") return a >= b;
                else std::cerr<< "error op: " << op <<std::endl;
            }
        } else if(v.type == "int"){
            if(types[0] != "int") assert(0);// type check failed
            int a = *(int *)item[offset[0]].c_str();
            int b = v.iVal;
            char a_null = *(item[offset[0]].c_str() + 4);
            if(a_null == 1) return false;
            if(op == "=") return a == b;
                else if (op == "<") return a < b;
                else if (op == ">") return a > b;
                else if (op == "<>") return a != b;
                else if (op == "<=") return a <= b;
                else if (op == ">=") return a >= b;
            else std::cerr<< "error op: " << op <<std::endl;
        } else if(v.type == "string"){
            if(types[0] != "string")assert(0);// type check failed
            std::string a(item[offset[0]].begin(), item[offset[0]].end() - 1);
            std::string b = v.strVal;
            char a_null = *(item[offset[0]].end() - 1);
            if(a_null == 1) return false;
            if(op == "=") return a == b;
                else if (op == "<") return a < b;
                else if (op == ">") return a > b;
                else if (op == "<>") return a != b;
                else if (op == "<=") return a <= b;
                else if (op == ">=") return a >= b;
            else std::cerr<< "error op: " << op <<std::endl;
        }
        std::cerr<< "error type: " << v.type <<std::endl;
        return false;
    };

 */
json Statement::exec() {
    if(type == ShowDbs){
        TableManager::getInstance(); // create base dir
        auto file_name_list = FileUtils::listFiles(TableManager::base_dir.c_str());
        return json({{"result" , file_name_list}});
    }else if(type == CreateDb){
        TableManager::getInstance()->createDB(ch[0]->strVal);
    }else if(type == DropDb){
        TableManager::getInstance()->DropDB(ch[0]->strVal);
    }else if(type == UseDb){
        if(!TableManager::getInstance()->changeDB(ch[0]->strVal, false))
            return json({{"result", "No such Database."}});
    }else if(type == ShowTables){
        if(TableManager::getInstance()->hasDB()){
            vector<string> ret;
            auto file_name_list = FileUtils::listFiles(TableManager::getInstance()->dir.c_str());
            for(auto & str : file_name_list)
                if(str.find("_") == str.npos && str != SYS_TABLE_NAME)
                {
                    ret.push_back(str);
                }
            return json({{"result", ret}});
        }else return json({{"result", "No Database was specified."}});
    }else if(type == CreateTable){
        if(TableManager::getInstance()->hasDB()){
            auto fl = std::dynamic_pointer_cast<FieldList>(ch[1]->getNode());
            fl->checkPrimaryKey();
            function<void(tinydbpp::Pager *)> writeScheme = [this, &fl](tinydbpp::Pager * ptr){
                auto p = ptr->getPage(0);
                char* buf = p->getBuf();
                char size_str[20];
                int num = 0;
                for(auto &f : fl->vec)
                    if(!f.is_primary_key_stmt)
                        num++;
                sprintf(size_str, "%d", num);
                string tmp = string("    ") + size_str + " ";
                for(auto &f : fl->vec){
                    if(f.is_primary_key_stmt) continue;
                    string pattern = f.type == "varchar"? "-1" : "5"; //bits to bytes +1 is null
                    sprintf(size_str, "%d", f.size);
                    //can null 0 not null 1
                    tmp = tmp + f.name + " " + f.type + " " + pattern + " " + ((f.is_key | !f.can_null)? string("1 "):string("0 "))
                          + (f.is_key? string("1 ") : string("0 ")) + size_str + " "; // unique

                    // create index here
                    auto indexName = TableManager::createIndexName(ch[0]->strVal, f.name);
                    if(f.is_key)
                        TheBTree::BT::setupBTree(indexName);
                }
                sprintf(buf, "%s" , tmp.c_str());
                p->markDirty();
                p->releaseBuf(buf);
            };
            TableManager::getInstance()->buildTable(ch[0]->strVal, writeScheme);
        }else return json({{"result", "No Database was specified."}});
    }else if(type == DropTable){
        if(!TableManager::getInstance()->dropTable(ch[0]->strVal))
            return json({{"result", "There's no this table."}});
    }else if(type == DesribeTable){
        auto td = TableManager::getInstance()->getTableDescription(ch[0]->strVal);
        if(td == nullptr) return json({{"result", "There's no this table."}});
        return json({{"result", {{"name", td->col_name}, {"type", td->col_type}, {"pattern", td->pattern},
                     {"not null", td->col_not_null}, {"unique", td->col_unique}, {"has index", td->col_has_index},{"width", td->col_width}} }});
    }else if(type == InsertItem){
        auto vlists = dynamic_pointer_cast<ValueLists>(ch[1]->getNode());
        auto td = TableManager::getInstance()->getTableDescription(ch[0]->strVal);
        for(auto & vlist : vlists->vec){
            if(vlist->vec.size() != td->col_name.size()) return json({{"result", "Wrong Dimension."}});
            vector<string> item;
            for(int i = 0;i < vlist->vec.size();i++) {
                string v_str(td->pattern[i] > 0? td->pattern[i] : 1, 0);
                auto v = vlist->vec[i];
                if(v->type == "NULL") {
                    if(td->col_not_null[i] == 1) return json({{"result", td->col_name[i] + "can not be null."}});
                    *(v_str.end() - 1) = 1;
                }
                else if(v->type == "int")
                    v_str.replace(v_str.begin(), v_str.begin() + 4, string((char*)&(v->iVal), (char*)(&(v->iVal)+ 4)));
                else if(v->type == "string")
                    v_str = v->strVal + "\0";
                item.push_back(v_str);
                //TODO special check
            }
            // check unique
            bool duplicated = false;
            for(int i = 0;i < td->col_name.size();i++)
                if(item[i].back() != 1)// null can duplicate
                if(td->col_has_index[i] == 1 && td->col_unique[i] == 1)
                {
                    TheBTree index(TableManager::getInstance()->dir + "/" + TableManager::createIndexName(td->name, td->col_name[i]));
                    auto vec = index.get(hash<string>()(item[i]));
                    for(auto & pre : vec)
                        if(pre == item[i]) {
                            duplicated = true;
                            break;
                        }
                    if(duplicated) break;
                }
            if(!duplicated) {
                bool fixed_res;
                string rec = td->embed(item, fixed_res);
                Location loc = RecordManager::getInstance()->insert(td->name, rec, fixed_res);
                for (int i = 0; i < td->col_name.size(); i++)
                    if (td->col_has_index[i] == 1) {
                        TheBTree index(TableManager::getInstance()->dir + "/" +
                                       TableManager::createIndexName(td->name, td->col_name[i]));
                        index.insert(std::hash<string>()(item[i]), TheBTree::BT::locationToString(loc), false);
                    }
            }
        }
    }else if (type == UpdateItem){

    }else if (type == DeleteItem){

    }else if (type == SelectItem){

    }else if (type == CreateIdx){
        //TODO 123
    }else if (type == DropIdx){

    }
    return nullptr;
}

Statement::~Statement() { }

Statement::Statement(Statement::Type type) :type(type) { }

TableManagement::~TableManagement() { }

TableManagement::TableManagement(Statement::Type type) :Statement(type) { }

Field::~Field() {}
