#pragma once

#include <BTree/BTree.h>

namespace tinydbpp {
/**
 * BTreePlus class.
 * A btree that allows duplicate keys.
 * @param KeyT: methods required:
 *              copy constructor, assignment operator, < operator
 *
 */
template<typename KeyT, size_t BRankMin = 2, size_t BRankMax = 3>
class BTreePlus {
public:
  typedef BTree<KeyT, BRankMin, BRankMax> BT;
  BTreePlus(const std::string tableName) :btree(tableName) { }

  std::vector<std::string> get(KeyT key) {
    std::string ptrs = btree.get(key);
    BOOST_ASSERT(ptrs.size() % LocLength == 0);
    int count = ptrs.size() / LocLength;
    std::vector<std::string> result;
    for (int i = 0; i < count; ++i) {
      Location loc = btree.stringToLocation(ptrs.substr(i * LocLength, LocLength));
      result.push_back(RecordManager::getInstance()->getRecord(btree.getTableName(), loc));
    }
    return result;
  }

  void insert(KeyT key, const std::string &data, bool fixed) {
    Location dataLoc = RecordManager::getInstance()->insert(btree.getTableName(), data, fixed);
    std::string strDataLoc = btree.locationToString(dataLoc);

    try {
      btree.update(key, [this, key, &strDataLoc](std::string &locs) -> bool {
        locs += strDataLoc;
        BOOST_LOG_TRIVIAL(info) << "BTree::insertNonUnique(), insert "
                                << locs.size() / LocLength
                                << "-th data of key \""
                                << key  << "\"";
        return true;
      });
    }
    catch (typename BT::KeyNotFound e) {
      BOOST_LOG_TRIVIAL(info) << "BTree::insertNonUnique(), insert first data of key \""
                              << key  << "\"";

      btree.insert(key, strDataLoc);
    }
  }

  void traverse(std::function<bool (KeyT key, std::string &data)> callback) {
    auto root = btree.getRoot();

    btree.traverseLeafNode(root, [this, &callback](KeyT key, std::string &data) -> bool {
      BOOST_ASSERT(data.size() % LocLength == 0);
      bool changed = false;
      for (int i = 0; i < (int)(data.size() / LocLength); ++i) {
        Location loc = btree.stringToLocation(data.substr(i * LocLength, LocLength));
        std::string record = RecordManager::getInstance()->getRecord(btree.getTableName(), loc);
        changed = changed || callback(key, record);
      }
      return changed;
    });
  }

  void remove(KeyT key, std::function<bool (const std::string &)> checker, bool fixed) {
    std::string ptrs = btree.get(key);

    btree.update(key, [this, key, fixed, &ptrs, &checker](std::string &recordData) -> bool {
      int count = ptrs.size() / LocLength;
      std::string newRecordData;
      for (int i = 0; i < count; ++i) {
        Location loc = btree.stringToLocation(ptrs.substr(i * LocLength, LocLength));
        auto record = RecordManager::getInstance()->getRecord(btree.getTableName(), loc);
        if (checker(record)) {
          RecordManager::getInstance()->delOneRecord(btree.getTableName(), loc.pageNumber, loc.loc, fixed);
        }
        else {
          newRecordData += btree.locationToString(loc);
        }
      }
      recordData = newRecordData;
      return true;
    });
  }

  void update(KeyT key, std::function<bool (std::string &)> callback) {
    btree.update(key, [this, key, &callback](std::string &ptrs) -> bool {
      bool ptrsUpdated = false;
      int count = ptrs.size() / LocLength;
      for (int i = 0; i < count; ++i) {
        Location loc = btree.stringToLocation(ptrs.substr(i * LocLength, LocLength));
        std::string record = RecordManager::getInstance()->getRecord(btree.getTableName(), loc);
        if (callback(record)) {
          Location newLoc = RecordManager::getInstance()->updateOneRecord(btree.getTableName(), loc, record, false);
          if (newLoc != loc) {
            ptrs.replace(i * LocLength, LocLength, btree.locationToString(newLoc).c_str(), LocLength);
            ptrsUpdated = true;
          }
        }
      }
      return ptrsUpdated;
    });
  }
private:
  BT btree;
};

typedef BTreePlus<size_t, 2, 3> TheBTree;
}
