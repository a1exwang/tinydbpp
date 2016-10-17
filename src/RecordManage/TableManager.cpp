//
// Created by chord on 16/10/14.
//

#include <TableManager.h>
using namespace tinydbpp;
using namespace std;
TableManager * TableManager::ins = NULL;
std::string TableManager::dir;
shared_ptr<Pager> TableDescription::getPager(Pager::OpenFlag flag ){
    if(my_pager == nullptr)
        return my_pager = shared_ptr<Pager>(new Pager(path, flag));
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
    std::string ret;
    fixed_res = true;
    for(int i = 0;i < list.size();i++){
        if(pattern[i] == -1){
            if(list[i].size() > DEFAULT_VARCHAR_LEN)
                fixed_res = false;
            int len = (int)list[i].length();
            char tmp[5];
            *(int*)tmp = len;
            tmp[4] = '\0';
            ret += string(tmp);
        }
        ret += list[i];
    }
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
    ret->path = dir + "/" + dbname + "/" + name;
    auto p = ret->getPager()->getPage(0);
    //TODO parse schema
    ret->addPattern(4);
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

bool TableManager::isExist(std::string name) {
    //TODO check info in DBTable
    string whole_name = TableManager::getInstance()->dir + "/" +TableManager::getInstance()->dbname + "/" + name;
    FILE* testFile = fopen(whole_name.c_str(), "r");
    if(testFile == NULL) return false;
    fclose(testFile);
    return true;
}

bool TableManager::buildTable(std::string name) {
    if(isExist(name)) return false;
    else{
        string whole_name = dir + "/" + dbname + "/" + name;
        shared_ptr<Pager> ptr( new Pager(whole_name, Pager::ReadWrite) );
        shared_ptr<Page> p = ptr->getPage(0);
        //TODO write scheme
        return true;
    }
}
