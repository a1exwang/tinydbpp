//
// Created by chord on 16/10/14.
//

#include "TableManager.h"
using namespace tinydbpp;
using namespace std;
std::shared_ptr<Pager> TableDescription::getPager(Pager::OpenFlag flag = Pager::ReadWrite){
    if(my_pager == nullptr)
        return my_pager = new Pager(TableManager::getInstance()->dir + "/" +TableManager::getInstance()->dbname + getRelativePath(), flag);
    else return my_pager;
}
std::shared_ptr<TableDescription> TableManager::getTableDescription(std::string name) {
    for(auto ptr : table_map)
    if(ptr->name == name)
        return ptr;
    if(!isExist(name))
        return nullptr;
    shared_ptr<TableDescription> ret = new TableDescription();
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
