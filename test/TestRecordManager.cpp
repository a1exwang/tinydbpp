//
// Created by chord on 16/10/18.
//

#include <Pager.h>
#include <RecordManage/RecordManager.h>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <Page.h>
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
        shared_ptr<Page> p = td->getPager()->getPage(loc.pageNumber);
        char * buf = p->getBuf();
        REQUIRE(buf[loc.loc] == 1);
        REQUIRE(string(buf + loc.loc + 1, res.length()) == res);
//        for(int i = 0;i < res.length();i++)
//            cout << (int)res[i]<< " " <<endl;
        //p->writeBack();
        p->releaseBuf(buf);
    }
}




