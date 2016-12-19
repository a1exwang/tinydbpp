
#include "Nodes.h"
#include <Parser/ParserVal.h>
#include <cassert>
#include <iostream>
using namespace tinydbpp::ast;

std::shared_ptr<const tinydbpp::ParserVal> SysManagement::getTarget() const {
  return target;
}

void Statements::addStatementToFront(std::shared_ptr<Statement> stmt) {
  statements.insert(statements.begin(), stmt);
}

void whereClause::becomeCompare(const std::string &colname, const std::string &op, Value v) {
    names.push_back(colname);
    if(v.type == "col")
        names.push_back(v.strVal);
    func = [op, v](const std::vector<std::string>& item, const std::vector<int>& offset,const std::vector<std::string>& types){
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

}

void whereClause::becomeIsNull(const std::string &colname) {
    names.push_back(colname);
    func = [](const std::vector<std::string> &item, const std::vector<int> &offset,
              const std::vector<std::string> &types) {
        char a_null = *(item[offset[0]].end() - 1);
        return a_null > 0;
    };
}

void whereClause::becomeIsNotNull(const std::string &colname) {
    names.push_back(colname);
    func = [](const std::vector<std::string> &item, const std::vector<int> &offset,
              const std::vector<std::string> &types) {
        char a_null = *(item[offset[0]].end() - 1);
        return a_null == 0 || a_null == 2;
    };
}

void whereClause::becomeAnd(std::shared_ptr<whereClause> w1, std::shared_ptr<whereClause> w2) {
    names = w1->names;
    for(auto & str : w2->names)
        names.push_back(str);

}
