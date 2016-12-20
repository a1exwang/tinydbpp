
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
void Statement::exec() {
    if(type == ShowDbs){
        TableManager::getInstance(); // create base dir
        auto file_name_list = FileUtils::listFiles(TableManager::base_dir.c_str());
        for(auto & str : file_name_list)
            cout << str << " ";
        cout <<endl;
    }else if(type == CreateDb){
        TableManager::getInstance()->createDB(ch[0]->strVal);
    }else if(type == DropDb){
        TableManager::getInstance()->DropDB(ch[0]->strVal);
    }else if(type == UseDb){
        if(!TableManager::getInstance()->changeDB(ch[0]->strVal, false))
            cout << "No such Database."<<endl;
    }else if(type == ShowTables){
        if(TableManager::getInstance()->hasDB()){

        }else cout << "No Database was specified."<<endl;
    }else if(type == CreateTable){
        if(TableManager::getInstance()->hasDB()){
            auto fl = std::dynamic_pointer_cast<FieldList>(ch[1]->getNode());
            fl->checkPrimaryKey();
            function<void(tinydbpp::Pager *)> writeScheme = [this, &fl](tinydbpp::Pager * ptr){
                auto p = ptr->getPage(0);
                char* buf = p->getBuf();
                char size_str[20];
                sprintf(size_str, "%d", (int)fl->vec.size());
                string tmp = string("    ") + size_str + " ";
                for(auto &f : fl->vec){
                    if(f.is_primary_key_stmt) continue;
                    int k = f.type == "varchar"? -1 : ((f.size + 3) / 4);//bits to
                    sprintf(size_str, "%d", k);
                    //can null 0 not null 1
                    tmp = tmp + f.name + " " + f.type + " " + size_str + " " + ((f.is_key | !f.can_null)? string("1 "):string("0 "))
                          + (f.is_key? string("1 ") : string("0 ")); // unique

                    // DONE create index here
                    auto indexName = TableManager::createIndexName(ch[0]->strVal, f.name);
                    TheBTree::BT::setupBTree(indexName);
                }
                sprintf(buf, "%s" , tmp.c_str());
                p->markDirty();
                p->releaseBuf(buf);
            };
            TableManager::getInstance()->buildTable(ch[0]->strVal, writeScheme);
        }else cout << "No Database was specified."<<endl;
    }
}

Statement::~Statement() { }

Statement::Statement(Statement::Type type) :type(type) { }

TableManagement::~TableManagement() { }

TableManagement::TableManagement(Statement::Type type) :Statement(type) { }

Field::~Field() {}
