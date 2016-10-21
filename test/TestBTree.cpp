#include "TestUtils.h"
#include <Pager.h>
#include <Page.h>
#include <BTree/BTree.h>
#include <memory>

/* Must define this MACRO to prevent from linking error. */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BTree
#include <boost/test/unit_test.hpp>

using namespace tinydbpp;
using namespace std;

TestUtils utils;
constexpr size_t MIN = 2;
constexpr size_t MAX = 3;

BOOST_AUTO_TEST_CASE(insert) {
  TableManager::getInstance()->changeDB("Test");
  auto indexName = "Index1";
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  string data;
  uint32_t key = 1;

  stringstream ss;
  ss << key;
  ss >> data;

  btree.insert(key, data);

  auto newData = btree.get(key);
  BOOST_REQUIRE(newData == data);
}


