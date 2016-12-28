#include "TestUtils.h"
#include <Pager/Pager.h>
#include <Pager/Page.h>
#include <BTree/BTreePlus.h>
#include <memory>

/* Must define this MACRO to prevent from linking error. */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BTree
#include <boost/test/unit_test.hpp>

using namespace tinydbpp;
using namespace std;

TestUtils utils;
constexpr size_t MIN = 3;
constexpr size_t MAX = 5;
typedef BTreePlus<uint32_t, MIN, MAX> TBTree;
BOOST_AUTO_TEST_CASE(insertBtreeOneKey) {
  TableManager::getInstance()->changeDB("Test");
  string tableName = "insertBtreePlusOneKey";
  boost::filesystem::remove("database/Test/" + tableName);
  TBTree::BT::setupBTree(tableName);

  TBTree btree(tableName);

  btree.insert(1, "1A", false);
}

BOOST_AUTO_TEST_CASE(insertBtreeMultipleKey) {
  TableManager::getInstance()->changeDB("Test");
  string tableName = "insertBtreePlusMultipleKey";
  boost::filesystem::remove("database/Test/" + tableName);
  TBTree::BT::setupBTree(tableName);

  TBTree btree(tableName);

  btree.insert(1, "1A", false);
  btree.insert(1, "1B", false);
  btree.insert(1, "1C", false);
  btree.insert(1, "1D", false);
  btree.insert(1, "1E", false);
  int cnt = 0;
  btree.traverse([&cnt](uint32_t key, string &data) -> bool {
    BOOST_REQUIRE(key == 1);
    stringstream ss;
    ss << "1" << (char)('A' + cnt);
    BOOST_REQUIRE_EQUAL(data, ss.str());
    cnt++;
    return false;
  });
}

BOOST_AUTO_TEST_CASE(insertBtreeMultipleKeys) {
  TableManager::getInstance()->changeDB("Test");
  string tableName = "insertBtreeMultipleKeys";
  boost::filesystem::remove("database/Test/" + tableName);
  TBTree::BT::setupBTree(tableName);

  TBTree btree(tableName);

  btree.insert(1, "1A", false);
  btree.insert(1, "1B", false);
  btree.insert(1, "1C", false);
  btree.insert(2, "2C", false);
  btree.insert(2, "2C", false);
  btree.insert(2, "2C", false);
  btree.insert(1, "1D", false);
  btree.insert(1, "1E", false);
  int cnt = 0;
  btree.traverse([&cnt](uint32_t key, string &data) -> bool {
    if (key == 1) {
      stringstream ss;
      ss << "1" << (char)('A' + cnt);
      BOOST_REQUIRE_EQUAL(data, ss.str());
      cnt++;
    }
    return false;
  });
}

BOOST_AUTO_TEST_CASE(deleteBtreeMultipleKeys) {
  TableManager::getInstance()->changeDB("Test");
  string tableName = "deleteBtreeMultipleKeys";
  boost::filesystem::remove("database/Test/" + tableName);
  TBTree::BT::setupBTree(tableName);

  TBTree btree(tableName);

  btree.insert(1, "1A", false);
  btree.insert(1, "1B", false);
  btree.insert(1, "1C", false);
  btree.insert(1, "1D", false);
  btree.insert(1, "1E", false);
  btree.remove(1, [](const std::string &record) -> bool {
    return record == "1A";
  }, false);

  int cnt = 0;
  btree.traverse([&cnt](uint32_t key, string &data) -> bool {
    if (key == 1) {
      stringstream ss;
      ss << "1" << (char)('A' + cnt + 1);
      BOOST_REQUIRE_EQUAL(data, ss.str());
      cnt++;
    }
    return false;
  });
  BOOST_ASSERT(cnt == 4);
}

BOOST_AUTO_TEST_CASE(getBtreeMultipleKeys) {
  TableManager::getInstance()->changeDB("Test");
  string tableName = "getBtreeMultipleKeys";
  boost::filesystem::remove("database/Test/" + tableName);
  TBTree::BT::setupBTree(tableName);

  TBTree btree(tableName);

  btree.insert(1, "1A", false);
  btree.insert(1, "1B", false);
  btree.insert(1, "1C", false);
  btree.insert(1, "1D", false);
  btree.insert(1, "1E", false);
  auto dataVec = btree.get(1);

  int cnt = 0;
  for (auto data : dataVec) {
    stringstream ss;
    ss << "1" << (char) ('A' + cnt);
    BOOST_REQUIRE_EQUAL(data, ss.str());
    cnt++;
  }
  BOOST_ASSERT(cnt == 5);
}

BOOST_AUTO_TEST_CASE(updateBtreeMultipleKeys) {
  TableManager::getInstance()->changeDB("Test");
  string tableName = "updateBtreeMultipleKeys";
  boost::filesystem::remove("database/Test/" + tableName);
  TBTree::BT::setupBTree(tableName);

  TBTree btree(tableName);

  btree.insert(1, "1A", false);
  btree.insert(1, "1B", false);
  btree.insert(1, "1C", false);
  btree.insert(1, "1D", false);
  btree.insert(1, "1E", false);
  btree.update(1, [](std::string &record) -> bool {
    record[1] += 1;
    return true;
  });

  int cnt = 0;
  btree.traverse([&cnt](uint32_t key, string &data) -> bool {
    if (key == 1) {
      stringstream ss;
      ss << "1" << (char)('A' + cnt + 1);
      BOOST_REQUIRE_EQUAL(data, ss.str());
      cnt++;
    }
    return false;
  });
  BOOST_ASSERT(cnt == 5);
}
