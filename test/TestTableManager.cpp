//
// Created by chord on 16/10/17.
//

#include "TestTableManager.h"
#include <Pager.h>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <RecordManage/TableManager.h>

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
    }
}

