//
// Created by chord on 16/10/14.
//

#include "TableManager.h"
using namespace tinydbpp;

std::shared_ptr<TableDescription> TableManager::getTableDescription(std::string name) {
    for(auto ptr : table_map)
    if(ptr->name == name)
        return ptr;
    //TODO
    /*
     * 1.traverse the file sysTable
     * 2.if the row's name == input name
     * 3.parse the other string and store it in map then return;
     * 4.return null
     */

}
