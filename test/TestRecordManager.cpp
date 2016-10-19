//
// Created by chord on 16/10/18.
//

#include <Pager.h>
#include <RecordManage/RecordManager.h>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <RecordManage/TableManager.h>
#include <vector>
#include <iostream>
using namespace tinydbpp;
using namespace std;
TEST_CASE("correctness","[RecordManager]"){
    TableManager::getInstance()->changeDB("Test");
    TableManager::getInstance()->buildTable("Number");
    shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription("Number");
    SECTION("Insert"){
        td->addPattern(-1);
        vector<string> a = {"abcd", "rfs3efc34"}, b;
        bool fixed;
        string res = td->embed(a, fixed);
        auto loc = RecordManager::getInstance()->insert("Number", res, fixed);
        cout << "Location: " << loc.pageNumber << " "<<loc.loc <<endl;
    }
}




