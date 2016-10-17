//
// Created by chord on 16/10/14.
//

#include "TableManager.h"
using namespace tinydbpp;
using namespace std;
TableManager * TableManager::ins = NULL;

shared_ptr<Pager> TableDescription::getPager(Pager::OpenFlag flag ){
    if(my_pager == nullptr)
        return my_pager = shared_ptr<Pager>(new Pager(TableManager::getInstance()->dir + "/" +TableManager::getInstance()->dbname + getRelativePath(), flag));
    else return my_pager;
}

std::vector<std::string> TableDescription::read(char* buf, int len, int& now){
    auto ret = std::vector<std::string>();
    for(int x : pattern){
        if(x > 0){
            BOOST_ASSERT(now + x <= len);
            ret.push_back(std::string(buf + now, x));
            now += x;
        }else if(x == -1){
            BOOST_ASSERT(now + 4 <= len);
            int next_len = *(int*)(std::string(buf + now, 4).c_str());
            now += 4;
            BOOST_ASSERT(now + next_len <= len);
            ret.push_back(std::string(buf + now, next_len));
        }else
            BOOST_ASSERT(0);
    }
    return ret;
}
std::string TableDescription::embed(const std::vector<std::string> list, bool & fixed_res){
    //TODO
    std::string ret;
    for(auto s : list)
        ret += s;
    fixed_res = false;
    return ret;
}



std::shared_ptr<TableDescription> TableManager::getTableDescription(std::string name) {
    for(auto ptr : table_map)
    if(ptr->name == name)
        return ptr;
    if(!isExist(name))
        return nullptr;
    shared_ptr<TableDescription> ret(new TableDescription());
    ret->name = name;
    ret->addPattern(4);
    auto p = ret->getPager()->getPage(0);
    //TODO parse schema
    table_map.push_back(ret);
    return ret;
}

void TableManager::changeDB(std::string db) {
    if(db == dbname)return ;
    this->dbname = db;
    if(ins){
        table_map.clear();
    }
}

bool TableManager::isExist(std::string basic_string) {
    return false;
}
