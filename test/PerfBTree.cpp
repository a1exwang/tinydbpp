#include "TestUtils.h"
#include <Pager.h>
#include <Page.h>
#include <PagerPerfMon.h>
#include <BTree/BTree.h>
#include <memory>


using namespace tinydbpp;
using namespace std;

TestUtils utils;
constexpr size_t MIN = 2;
constexpr size_t MAX = 3;
typedef BTree<uint32_t, MIN, MAX> TBTree;

int main() {
  TableManager::getInstance()->changeDB("Test");
  string indexName = "PerfmIndex";
  boost::filesystem::remove("database/Test/" + indexName);
  BTree<uint32_t, MIN, MAX>::setupBTree(indexName);
  BTree<uint32_t, MIN, MAX> btree(indexName);

  for (int i = 0; i < 100; ++i) {
    stringstream ss;
    ss << setfill('0') << setw(4) << i;
    btree.insert(i, ss.str());
  }

  const PagerPerfMon *perfMon = btree.getPager()->getPerfMon();
  cout << perfMon->report() << endl;

  return 0;
}
