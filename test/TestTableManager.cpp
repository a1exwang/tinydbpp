//
// Created by chord on 16/10/17.
//

#include <Pager.h>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <RecordManage/TableManager.h>
#include <vector>
#include <iostream>
using namespace tinydbpp;
using namespace std;


TEST_CASE("getTableDescription","[TableManager]"){
    TableManager::getInstance()->changeDB("Test");
    SECTION("new Table"){
        TableManager::getInstance()->buildTable("Number");
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription("Number");
        REQUIRE(td->len == 4);
        REQUIRE(td->name == "Number");
        REQUIRE(td->getPath() == string(DEFAULT_DATABASE_DIR) + "/Test/Number");
        td->addPattern(-1);
        vector<string> a = {"abcd", "rfs3efc34"}, b;
        bool fixed;
        string res = td->embed(a, fixed);
        int now = 0;
        char tmp[100];
        memcpy(tmp, res.c_str(), res.length());
        b = td->read(tmp, res.length(), now, fixed);
        REQUIRE(b.size() == a.size());
        for(int i = 0;i < a.size();i++) {
            REQUIRE(a[i] == b[i]);
        }
        REQUIRE(now == DEFAULT_VARCHAR_LEN + 8);
    }
    SECTION("do not exist"){
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription("NOP");
        REQUIRE(td == nullptr);
    }
}

