//
// Created by chord on 16/10/14.
//

#include <Pager.h>
#include "RecodeManager.h"
#include "TableManager.h"

using namespace std;
namespace tinydbpp {
    Location RecodeManager::insert(const std::string &table_name, const std::string &record) {
        shared_ptr<Pager> ptr = TableManager::getInstance()->getTableDescription(table_name)->getPager();
        shared_ptr<Page> dic_page = ptr->getPage(1);
        char* dic = dic_page->getBuf();
        dic_page->markDirty();
        if
    }

    void RecodeManager::update(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                               std::function<void(std::vector<std::string>, char *)>) {

    }

    void RecodeManager::del(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                            std::function<void(std::vector<std::string>, char *)>) {

    }

    void RecodeManager::select(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                               std::function<void(std::vector<std::string>, char *)>) {

    }
}

