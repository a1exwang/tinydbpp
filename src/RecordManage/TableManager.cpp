//
// Created by chord on 16/10/14.
//

#include <TableManager.h>
#include <iostream>
#include <sstream>
using namespace tinydbpp;
using namespace std;
TableManager * TableManager::ins = NULL;
TableManager::Garbo TableManager::garbo;
std::string TableManager::base_dir;
shared_ptr<Pager> TableDescription::getPager(Pager::OpenFlag flag ){
    if(my_pager == nullptr)
        return my_pager = shared_ptr<Pager>(new Pager(path, flag));
    else return my_pager;
}

std::vector<std::string> TableDescription::read(char* buf, int len, int& now, bool fixed){
    auto ret = std::vector<std::string>();
    int minimum = now + this->len;
    for(int x : pattern){
        if(x > 0){
            BOOST_ASSERT(now + x <= len);
            ret.push_back(std::string(buf + now, x));
            now += x;
        }else if(x == -1){
            BOOST_ASSERT(now + 4 <= len);
            int next_len = *(int*)(buf + now);
            now += 4;
            BOOST_ASSERT(now + next_len <= len);
            ret.push_back(std::string(buf + now, buf + now + next_len));
        }else
            BOOST_ASSERT(0);
    }
    if(fixed && now < minimum)
        now = minimum;
    return ret;
}

void TableDescription::addPattern(int x){
    pattern.push_back(x);
    if(x == -1) len += 4 + DEFAULT_VARCHAR_LEN;
    else len += x;
}
std::string TableDescription::embed(const std::vector<std::string> list, bool & fixed_res){
    std::string ret;
    fixed_res = true;
    for(uint32_t i = 0;i < list.size();i++){
        if(pattern[i] == -1) {
            if (list[i].size() > DEFAULT_VARCHAR_LEN)
                fixed_res = false;
            int len = (int) list[i].length();
            char tmp[4];
            *(int *) tmp = len;
            ret += string(tmp, tmp + 4);
        }
        ret += list[i];
    }
    if(fixed_res)
        ret.resize(len);
    return ret;
}



std::shared_ptr<TableDescription> TableManager::getTableDescription(std::string name) {
    for(auto ptr : table_map)
        if(ptr->name == name)
            return ptr;
    if(!FileUtils::isExist((dir + "/" + name).c_str()))
        return nullptr;
    shared_ptr<TableDescription> ret(new TableDescription());
    ret->name = name;
    ret->path = dir + "/" + name;
    auto parse_func = [this, &name](shared_ptr<TableDescription> new_td){
        //0.this function should be implemented in parser module, this is a test func
        //1.read from page(0) in new_td->getPager()->getPage(0) and parse
        //2.add bytes pattern of new_td;
        //3.if necessary add other properties in TableDescription class(whether there is a index)
        auto p = new_td->getPager()->getPage(0);
        char* buf = p->getBuf();
        istringstream istr;
        istr.str(buf);
        int num;
        istr >> num;
        for(int i = 0;i < num;i++)
        {
            string colName,t;
            int pat, b1, b2, w;
            istr >> colName >> t >> pat >> b1 >> b2 >> w;
            new_td->col_name.push_back(colName);
            new_td->col_type.push_back(t);
            new_td->addPattern(pat);
            new_td->col_not_null.push_back(b1);
            new_td->col_unique.push_back(b2);
            new_td->col_width.push_back(w);
            // DONE check has index
            FileUtils fileUtils;
            int hasIndex = fileUtils.isExist((dir + "/" + createIndexName(name, colName)).c_str());
            new_td->col_has_index.push_back(hasIndex);
        }
        p->releaseBuf(buf);
    };
    if(name.find("_") == string::npos)// not index "table"
        parse_func(ret);
    table_map.push_back(ret);
    return ret;
}

bool TableManager::changeDB(std::string db, bool auto_create) {
    if(db == dbname)
        return true;
    string dbtable_name = this->base_dir + "/" + db + "/" + SYS_TABLE_NAME;
    if(!FileUtils::isExist(dbtable_name.c_str()))
        if(!auto_create)
            return false;
        else
            FileUtils::createFile(dbtable_name.c_str());
    this->dbname = db;
    this->dir = this->base_dir + "/" + db;
    table_map.clear();
    return true;
}

void TableManager::createDB(std::string db) {
    string dbtable_name = this->base_dir + "/" + db + "/" + SYS_TABLE_NAME;
    if(!FileUtils::isExist(dbtable_name.c_str()))
            FileUtils::createFile(dbtable_name.c_str());
}

bool TableManager::DropDB(std::string db) {
    if(dbname == db){
        dbname = "";
        table_map.clear();
    }
    return FileUtils::deleteDir((this->base_dir + "/" + db).c_str());
}

bool TableManager::buildTable(std::string name, std::function<void(Pager *)> callback) {
    string whole_name = dir + "/" + name;
    if(FileUtils::isExist(whole_name.c_str()))
        return false;
    else{
        shared_ptr<Pager> ptr( new Pager(whole_name, Pager::ReadWrite) );
        if (callback) {
            callback(ptr.get());
        }
        ptr->writeBackAll();
        return true;
    }
}

std::string TableManager::createIndexName(const std::string &tableName, const std::string &colName) {
    return tableName + "_" + colName;
}

bool TableManager::parseIndex(const std::string &indexName, const std::string &tableName, std::string &colName) {
    if (indexName.size() > tableName.size() && indexName.substr(0, tableName.size()) == tableName) {
        colName = indexName.substr(tableName.size() + 1, indexName.size() - (tableName.size() + 1));
        return true;
    }
    return false;
}

bool TableManager::dropTable(std::string name) {
    for(auto iter = table_map.begin();iter != table_map.end();)
        if((*iter)->name == name || ((*iter)->name).find(name + "_") != string::npos)
            table_map.erase(iter++);
        else iter ++;

    auto files = FileUtils::listFiles(dir.c_str());
    bool ret = false;
    for(auto & f : files)
        if(f == name || f.find(name + "_") != string::npos) {
            FileUtils::deleteFile((dir + "/" + f).c_str());
            ret = true;
        }
    return ret;
}
