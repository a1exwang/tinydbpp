#include "TestUtils.h"
#include <Pager/Pager.h>
#include <Pager/Page.h>
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
typedef BTree<uint32_t, MIN, MAX> TBTree;

BOOST_AUTO_TEST_CASE(insertRoot) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexRoot";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);
  BTree<uint32_t, MIN, MAX> btree(indexName);
  btree.insert(1, "1");
  BOOST_REQUIRE(btree.get(1)== "1");
}

BOOST_AUTO_TEST_CASE(insertLeaf) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexLeaf";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(2, "2");
  btree.insert(1, "1");
  BOOST_REQUIRE(btree.get(1)== "1");
}

BOOST_AUTO_TEST_CASE(insertGrow1) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexGrow1";
  boost::filesystem::remove("database/Test/" + indexName);
  TBTree::setupBTree(indexName);
  TBTree btree(indexName);

  btree.insert(3, "3");
  btree.insert(1, "1");
  btree.insert(2, "2");

  BOOST_REQUIRE(btree.get(1) == "1");
}
BOOST_AUTO_TEST_CASE(insertSplitInner) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexSplitInner";
  boost::filesystem::remove("database/Test/" + indexName);
  TBTree::setupBTree(indexName);
  TBTree btree(indexName);

  btree.insert(2, "2");
  btree.insert(9, "9");
  btree.insert(5, "5");
  btree.insert(1, "1");
  btree.insert(7, "7");
  btree.insert(4, "4");
  btree.insert(3, "3");
  btree.insert(6, "6");
  btree.insert(8, "8");

//  stringstream ss;
//  btree.dump(ss);
//  cout << ss.str();
  BOOST_REQUIRE(btree.get(1) == "1");
  BOOST_REQUIRE(btree.get(2) == "2");
  BOOST_REQUIRE(btree.get(7) == "7");
}

BOOST_AUTO_TEST_CASE(insertKeyDuplicated) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexInsertKeyDuplicated";
  boost::filesystem::remove("database/Test/" + indexName);
  TBTree::setupBTree(indexName);

  TBTree btree(indexName);

  btree.insert(1, "1");
  btree.insert(2, "2");
  btree.insert(3, "3");
  btree.insert(4, "4");
  btree.insert(5, "5");
  btree.insert(6, "6");
  btree.insert(7, "7");
  btree.insert(8, "8");
  btree.insert(9, "9");

  BOOST_REQUIRE_THROW(btree.insert(1, "?"), TBTree::KeyDuplicated);
}

BOOST_AUTO_TEST_CASE(getKeyNotFound) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexGetKeyNotFound";
  boost::filesystem::remove("database/Test/" + indexName);
  TBTree::setupBTree(indexName);

  TBTree btree(indexName);

  btree.insert(1, "1");
  btree.insert(2, "2");
  btree.insert(3, "3");
  btree.insert(4, "4");
  btree.insert(5, "5");
  btree.insert(6, "6");
  btree.insert(7, "7");
  btree.insert(8, "8");
  btree.insert(9, "9");

  BOOST_REQUIRE_THROW(btree.get(10), TBTree::KeyNotFound);
}

BOOST_AUTO_TEST_CASE(updateData) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexUpdateData";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(1, "1");
  btree.insert(2, "2");
  btree.insert(3, "3");
  btree.insert(4, "4");
  btree.insert(5, "5");
  btree.insert(6, "6");
  btree.insert(7, "7");
  btree.insert(8, "8");
  btree.insert(9, "9");

  btree.updateNoResize(1, [](std::string &data) -> bool {
    data = "?";
    return true;
  });

  BOOST_REQUIRE(btree.get(1) == "?");
}

BOOST_AUTO_TEST_CASE(traverseBTree) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexTraverseBTree";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(9, "9");
  btree.insert(1, "1");
  btree.insert(5, "5");
  btree.insert(6, "6");
  btree.insert(7, "7");
  btree.insert(2, "2");
  btree.insert(8, "8");
  btree.insert(3, "3");
  btree.insert(4, "4");

  uint32_t i = 1;
  btree.traverse([&i](uint32_t key, const std::string &data) -> void {
    BOOST_ASSERT(i == key);
    stringstream ss;
    ss << i;
    BOOST_ASSERT(ss.str() == data);
    i++;
  });
}

BOOST_AUTO_TEST_CASE(traverseBTreeUpdateNode) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "IndexTraverseBTreeUpdateNode";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(1, "1");
  btree.insert(6, "6");
  btree.insert(8, "8");
  btree.insert(3, "3");
  btree.insert(4, "4");
  btree.insert(2, "2");
  btree.insert(7, "7");
  btree.insert(5, "5");

  uint32_t i = 1;
  btree.traverse([&i](uint32_t key, std::string &data) -> bool {
    BOOST_ASSERT(i == key);
    stringstream ss;
    ss << i + 1;
    data = ss.str();
    i++;
    return true;
  });

  btree.traverse([&i](uint32_t key, const std::string &data) -> void {
    stringstream ss;
    ss << key + 1;
    BOOST_ASSERT(data == ss.str());
  });
}


BOOST_AUTO_TEST_CASE(deleteBTreeRoot) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "deleteBTreeRoot";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(1, "1");

  btree.remove(1);
  BOOST_REQUIRE_THROW(btree.get(1), TBTree::KeyNotFound);
}

BOOST_AUTO_TEST_CASE(deleteInBTreeRoot) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "deleteBTreeRoot";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(1, "1");
  btree.insert(2, "2");

  btree.remove(1);
  BOOST_REQUIRE_THROW(btree.get(1), TBTree::KeyNotFound);
}

BOOST_AUTO_TEST_CASE(deleteBTreeDataShrintBelly) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "deleteBTreeLeafOfARichParent";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(1, "1");
  btree.insert(2, "2");
  btree.insert(3, "3");

  btree.remove(2);
  BOOST_REQUIRE_THROW(btree.get(2), TBTree::KeyNotFound);
}

BOOST_AUTO_TEST_CASE(deleteBTreeDataDeleteOneChild) {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "deleteBTreeDataDeleteOneChild";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);

  BTree<uint32_t, MIN, MAX> btree(indexName);

  btree.insert(1, "1");
  btree.insert(2, "2");
  btree.insert(3, "3");
  btree.insert(4, "4");
  btree.insert(5, "5");
  btree.insert(6, "6");

  btree.remove(3);
  BOOST_REQUIRE_THROW(btree.get(3), TBTree::KeyNotFound);
}

//BOOST_AUTO_TEST_CASE(deleteBTreeNodeDecreaseHeight) {
//  TableManager::getInstance()->changeDB("Test");
//  string indexName = "deleteBTreeNodeDecreaseHeight";
//  boost::filesystem::remove("database/Test/" + indexName);
//  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);
//
//  BTree<uint32_t, MIN, MAX> btree(indexName);
//
//  btree.insert(1, "1");
//  btree.insert(2, "2");
//  btree.insert(3, "3");
//  btree.insert(4, "4");
//  btree.insert(5, "5");
//  btree.insert(6, "6");
//
//  btree.remove(1);
//  BOOST_REQUIRE_THROW(btree.get(1), TBTree::KeyNotFound);
//}

