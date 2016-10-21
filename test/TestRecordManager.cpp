//
// Created by chord on 16/10/18.
//

#include "TestUtils.h"
#include <Pager.h>
#include <RecordManage/RecordManager.h>
#include <Page.h>
#include <RecordManage/TableManager.h>
#include <vector>
#include <iostream>
#include <boost/log/trivial.hpp>
#include <functional>

using namespace tinydbpp;
using namespace std;

TestUtils &tu() {
  static TestUtils tu0;
  return tu0;
}

void testInsertRecord() {
  auto tableName = "Number1";
  TableManager::getInstance()->changeDB("Test");
  TableManager::getInstance()->buildTable(tableName);

  shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(tableName);
  BOOST_ASSERT(td != nullptr);
  td->addPattern(-1);
  vector<string> a = {"abcd", "rfs3efc34"}, b;
//  vector<string> a = {
//          "fixed-length",
//          "variable-length"
//          // FIXME: long record fails
////            "This is a very very VERY VERY long and UNICODE 哈哈 variable-length record."
//  };
  bool fixed;
  string res = td->embed(a, fixed);
  auto loc = RecordManager::getInstance()->insert(tableName, res, fixed);
  shared_ptr<Page> p = td->getPager()->getPage((Pager::PageID) loc.pageNumber);
  char *buf = p->getBuf();
  BOOST_ASSERT(buf[loc.loc] == 1);
  BOOST_ASSERT(string(buf + loc.loc + 1, res.length()) == res);
  p->releaseBuf(buf);
}

void testSelect() {
  TableManager::getInstance()->changeDB("Test");
  string dbName = "Number2";
  TableManager::getInstance()->buildTable(dbName);
  shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(dbName);
  td->addPattern(-1);
  vector<string> a = {"abcd", "rfs3efc34"};
  bool fixed;
  string res = td->embed(a, fixed);
  auto loc = RecordManager::getInstance()->insert(dbName, res, fixed);

  function<void(vector<string> &, int, int)> solver = [&loc, &a](std::vector<std::string>& records, int pageNumber, int offsetInPage) -> void {
    BOOST_LOG_TRIVIAL(info) << "RecordManager::select() solver, pageNumber = " << pageNumber << ", offset = " << offsetInPage;
    for (auto r : records)
      BOOST_LOG_TRIVIAL(info) << tu().hexdump(r);
    if (loc.pageNumber == pageNumber && loc.loc == offsetInPage) {
      BOOST_ASSERT(memcmp((void*)records[0].c_str(), (void*)a[0].c_str(), a[0].size()) == 0);
    }
  };
  function<bool(const vector<string> &)> checker = [](auto _) -> bool { return true; };
  RecordManager::getInstance()->select(dbName, checker, solver);
}

void testDeleteOneRecord() {
  string tableName = "Number3";
  TableManager::getInstance()->changeDB("Test");
  TableManager::getInstance()->buildTable(tableName);
  shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(tableName);
  td->addPattern(-1);
  vector<string> a = {"abcd", "rfs3efc34"};
  bool fixed;
  string res = td->embed(a, fixed);
  auto loc = RecordManager::getInstance()->insert(tableName, res, fixed);

  function<bool(const vector<string> &)> checkerDel = [](auto _) -> bool { return true; };
  function<void(const vector<string> &)> solverDel = [](auto _) -> void {
    BOOST_LOG_TRIVIAL(info) << "solverDel";
  };

  RecordManager::getInstance()->del(tableName, checkerDel, solverDel);

  function<void(vector<string> &, int, int)> solver = [&loc, &a](std::vector<std::string>& records, int pageNumber, int offsetInPage) -> void {
    BOOST_ASSERT_MSG(false, "Deleted records should not be returned.");
  };
  function<bool(const vector<string> &)> checker = [](auto _) -> bool { return true; };
  RecordManager::getInstance()->select(tableName, checker, solver);
}

void testUpdateOneFixedLengthRecord() {
  string tableName = "Number4";
  TableManager::getInstance()->changeDB("Test");
  TableManager::getInstance()->buildTable(tableName);
  shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(tableName);
  td->addPattern(-1);
  vector<string> a = {"abcd", "rfs3efc34"};
  bool fixed;
  string res = td->embed(a, fixed);
  auto loc = RecordManager::getInstance()->insert(tableName, res, fixed);

  function<bool(const vector<string> &)> checkerUpdate = [](auto _) -> bool { return true; };
  function<void(vector<string> &)> solverUpdate = [](vector<string> &record) -> void {
    for (auto it = record.begin(); it < record.end(); ++it) {
      if (*it == "abcd") {
        BOOST_LOG_TRIVIAL(info) << "changing \"abcd\" to \"Abcd\"";
        (*it)[0] = 'A';
      }
    }
  };
  RecordManager::getInstance()->update(tableName, checkerUpdate, solverUpdate);

  function<void(vector<string> &, int, int)> solver = [&loc, &a](std::vector<std::string>& records, int pageNumber, int offsetInPage) -> void {
    for (auto r : records)
      BOOST_LOG_TRIVIAL(info) << tu().hexdump(r);
    if (loc.pageNumber == pageNumber && loc.loc == offsetInPage) {
      BOOST_ASSERT(memcmp((void*)records[0].c_str(), "Abcd", a[0].size()) == 0);
    }
  };
  function<bool(const vector<string> &)> checker = [](auto _) -> bool { return true; };
  RecordManager::getInstance()->select(tableName, checker, solver);
}

void testMultipleOpsOnOneTable() {
  auto tableName = "Number5";
  TableManager::getInstance()->changeDB("Test");
  TableManager::getInstance()->buildTable(tableName);

  shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(tableName);
  BOOST_ASSERT(td != nullptr);
  td->addPattern(-1);
  vector<string> a = {"abcd", "rfs3efc34"};
  bool fixed;
  string res = td->embed(a, fixed);
  auto loc = RecordManager::getInstance()->insert(tableName, res, fixed);

  shared_ptr<Page> p = td->getPager()->getPage((Pager::PageID) loc.pageNumber);
  char *buf = p->getBuf();
  BOOST_ASSERT(buf[loc.loc] == 1);
  BOOST_ASSERT(string(buf + loc.loc + 1, res.length()) == res);
  p->releaseBuf(buf);

  vector<string> b = {"dcba", "rfs3efc34"};
  res = td->embed(b, fixed);
  loc = RecordManager::getInstance()->insert(tableName, res, fixed);

  p = td->getPager()->getPage((Pager::PageID) loc.pageNumber);
  buf = p->getBuf();
  BOOST_ASSERT(buf[loc.loc] == 1);
  BOOST_ASSERT(string(buf + loc.loc + 1, res.length()) == res);
  p->releaseBuf(buf);
}

int main() {
  testInsertRecord();
  testSelect();
  testDeleteOneRecord();
  testUpdateOneFixedLengthRecord();
  testMultipleOpsOnOneTable();
}
