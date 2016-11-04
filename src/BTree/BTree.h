#pragma once

#include <RecordManage/RecordManager.h>
#include <RecordManage/TableManager.h>
#include <Pager.h>
#include <string>
#include <exception>
#include <functional>
#include <sstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <FileUtils.h>
#include <Page.h>

namespace tinydbpp {
constexpr uint32_t BTREE_FILE_MAGIC_NUMBER = 0xdb0b42ee;

/**
 * BTree class.
 * @param KeyT: methods required:
 *              copy constructor, assignment operator, < operator
 *
 */
template <typename KeyT, size_t BRankMin = 2, size_t BRankMax = 3>
class BTree {
  class BTreeError :public std::exception {
  public:
    BTreeError(const std::string &msg) :sMsg(msg) { }
    virtual ~BTreeError() { }
    virtual std::string toString() const {
      return sMsg;
    }
  protected:
    std::string sMsg;
  };
  class KeyNotFound :public BTreeError {
  public:
    KeyNotFound(KeyT key, const std::string &msg) :BTreeError(msg), key(key) { }
    virtual ~KeyNotFound() { }
    virtual std::string toString() const override {
      std::stringstream ss;
      ss << "KeyNotFound: " << key << std::endl;
      ss << BTreeError::sMsg;
      return ss.str();
    }
  private:
    KeyT key;
  };
  class KeyDuplicated :public BTreeError {
  public:
    KeyDuplicated(KeyT key, const std::string &msg) :BTreeError(msg), key(key) { }
    virtual ~KeyDuplicated() { }
    virtual std::string toString() const override {
      std::stringstream ss;
      ss << "KeyDuplicated: " << key << std::endl;
      ss << BTreeError::sMsg;
      return ss.str();
    }
  private:
    KeyT key;
  };

  friend class Node;
  class Node {
  public:
    /**
     * Init as a intermediate node
     * @param btree
     * @param parent
     * @param key
     * @return
     */
    Node(BTree &btree, std::shared_ptr<Node> parent, KeyT key)
            :btree(btree), parent(parent), loc(0, 0), isLeaf(false) {
      keys.push_back(key);
    }
    /**
     * Init as a leaf node
     * @param btree
     * @param parent
     * @param key
     * @param dataLoc
     * @return
     */
    Node(BTree &btree, std::shared_ptr<Node> parent, KeyT key, Location dataLoc)
            :btree(btree), parent(parent), loc(0, 0), isLeaf(true) {
      keys.push_back(key);
      children.push_back(dataLoc);
    }
    /**
     * Init from buffer
     * @param btree
     * @param parent
     * @param loc
     * @param buffer
     * @return
     */
    Node(BTree &btree, std::shared_ptr<Node> parent, Location loc, const std::string &buffer)
            :btree(btree), parent(parent), loc(loc) {
      initFromBuf(buffer);
    }
    void setLocation(Location loc) {
      this->loc = loc;
    }
    Location getLocation() const {
      return loc;
    }
    uint32_t getChildCount() const {
      return (uint32_t)children.size();
    }
    KeyT getKey(size_t i) const {
      BOOST_ASSERT(i < keys.size());
      return keys[i];
    }
    uint32_t getKeyCount() const {
      return (uint32_t)keys.size();
    }
    bool isLeafNode() const { return isLeaf; }
    std::shared_ptr<Node> getParent() {
      return parent;
    }
    std::shared_ptr<const Node> getParent() const {
      return parent;
    }
    /**
     * Insert a data node to the leaf node.
     * NOTE: This function only modify the buffer,
     *  you need to manually call writeBack() to write to disk.
     * @param at: position to insert
     */
    void insertLocDataInLeafNode(uint32_t at, KeyT key, Location locData) {
      BOOST_ASSERT(isLeaf);
      keys.insert(keys.begin() + at, key);
      children.insert(children.begin() + at, locData);
    }

    void insertNodeAt(size_t at, std::shared_ptr<Node> node) {
      // TODO: offset by 1 error ?
      keys.insert(keys.begin() + at + 1, node->getKey(0));
      children.insert(children.begin() + at + 2, node->getLocation());
    }

    /**
     * Node format
     * +00  u8    nKeyCount
     * +01  KeyT  keys[BRankMax-1]
     * +01+sizeof(KeyT)*(BRankMax-1)
     *      CompressedLoc children[BRankMax]
     *
     * CompressedLoc
     *   u32 pgNo
     *   u16 pgOff
     *
     *  Totol size: 1+sizeof(KeyT) * (B-1) + 6*B
     *    for KeyT = u32, total size is (10B-3), max B ~= 400
     *    for keyT = u64, total size is (14B-3), max B ~= 290
     *  It's enough!
     * @return
     */
#pragma pack(1)
    struct _NodeFormat {
      uint8_t nKeyCount;
      uint8_t isLeaf;
      uint32_t parentPgNo;
      uint16_t parentPgOff;
      KeyT keys[BRankMax - 1];
      struct {
        uint32_t pgNo;
        uint16_t pgOff;
      } children[BRankMax];
    };
#pragma pack()
//    /**
//     * _NodeFormat is larger than _LeafNodeFormat
//     */
//    struct _LeafNodeFormat {
//      uint8_t nKeyCount;
//      uint8_t isLeaf;
//      uint32_t parentPgNo;
//      uint16_t parentPgOff;
//      KeyT keys[BRankMax - 1];
//      struct {
//        uint32_t pgNo;
//        uint16_t pgOff;
//      } data[BRankMax - 1];
//    };
    std::string toBuf() const {
      _NodeFormat nf;
      memset(&nf, 0, sizeof(typename Node::_NodeFormat));

      BOOST_ASSERT(keys.size() <= std::numeric_limits<uint8_t>::max());
      nf.nKeyCount = static_cast<uint8_t>(keys.size());

      if (parent != nullptr) {
        nf.parentPgNo = static_cast<uint32_t>(getParent()->getLocation().pageNumber);
        auto pgOff = getParent()->getLocation().loc;
        BOOST_ASSERT(pgOff < (int)PAGER_PAGE_SIZE);
        nf.parentPgOff = static_cast<uint16_t>(pgOff);
      }
      else {
        nf.parentPgNo = nf.parentPgOff = 0;
      }

      nf.isLeaf = this->isLeaf ? (uint8_t)1 : (uint8_t)0;

      if (isLeaf) {
        for (uint32_t i = 0; i < keys.size(); ++i) {
          nf.keys[i] = keys[i];
          nf.children[i].pgNo = static_cast<uint32_t>(children[i].pageNumber);
          BOOST_ASSERT(children[i].loc <= (int)PAGER_PAGE_SIZE);
          nf.children[i].pgOff = static_cast<uint16_t>(children[i].loc);
        }
      }
      else {
        for (uint32_t i = 0; i < keys.size(); ++i) {
          nf.keys[i] = keys[i];
          nf.children[i].pgNo = static_cast<uint32_t>(children[i].pageNumber);
          BOOST_ASSERT(children[i].loc <= (int)PAGER_PAGE_SIZE);
          nf.children[i].pgOff = static_cast<uint16_t>(children[i].loc);
        }
        /**
         * Do not forget the last one.
         */
        nf.children[nf.nKeyCount].pgNo = static_cast<uint32_t>(children[nf.nKeyCount].pageNumber);
        nf.children[nf.nKeyCount].pgOff = static_cast<uint16_t>(children[nf.nKeyCount].loc);
      }

      return std::string((char*)&nf, sizeof(nf));
    }
    void writeBack() {
      BOOST_ASSERT(keys.size() + !isLeaf == children.size());
      /**
       * This node is newly created.
       */
      if (loc.pageNumber == 0) {
        std::string data = toBuf();
        this->loc = RecordManager::getInstance()->insert(this->btree.sTableName, data, /* fixed-length*/false);
        BOOST_LOG_TRIVIAL(info) << "BTree::Node::writeBack(). Created at loc = (" << loc.pageNumber << ", " << loc.loc << ")";
      }
      else {
        auto page = btree.pPager->getPage((uint32_t)this->loc.pageNumber);
        BOOST_LOG_TRIVIAL(info) << "BTree::Node::writeBack(). Updated at loc = (" << loc.pageNumber << ", " << loc.loc << ")";

        RecordManager::getInstance()->updateRecordNoResize(this->btree.sTableName, this->loc, [this](std::string &record) -> bool {
          record = toBuf();
          return true;
        });
      }
    }

    void splitNode() {
      BOOST_ASSERT(getKeyCount() == BRankMax);
      if (isLeaf) {
        size_t newLeftNodeSize = BRankMax / 2;
//        size_t newRightNodeSize = BRankMax - newLeftNodeSize;
        auto itRightKeysBegin = keys.begin() + newLeftNodeSize;
        auto itRightKeysEnd = keys.end();
        auto itRightChildrenBegin = children.begin() + newLeftNodeSize;
        auto itRightChildrenEnd = children.end();

        /**
         * Create right node
         */
        auto ret = std::shared_ptr<Node>(new Node(btree, nullptr, 0));
        ret->isLeaf = true;
        ret->keys = std::vector<KeyT>(itRightKeysBegin, itRightKeysEnd);
        ret->children = std::vector<Location>(itRightChildrenBegin, itRightChildrenEnd);
        ret->writeBack();

        keys.erase(itRightKeysBegin, itRightKeysEnd);
        children.erase(itRightChildrenBegin, itRightChildrenEnd);
        this->writeBack();
        if (parent == nullptr) {
          /**
           * Root node is full. We need a new root node.
           */
          auto newRoot = std::shared_ptr<Node>(new Node(btree, nullptr, *itRightKeysBegin));
          newRoot->isLeaf = false;
          newRoot->children.push_back(this->loc);
          newRoot->children.push_back(ret->loc);
          newRoot->writeBack();

          /**
           * Since new root node is created, we update his two children's parentLoc
           */
          this->parent = newRoot;
          this->writeBack();
          ret->parent = newRoot;
          this->writeBack();
          btree.storeRootLocation(newRoot->loc);
        }
        else {
          auto pos = ret->getParent()->getChildPos(this->getKey(0));
          ret->getParent()->insertNodeAt(pos, ret);
          if (parent->getKeyCount() >= BRankMax) {
            parent->splitNode();
          }
        }
      }
      else {
        BOOST_ASSERT(0);
      }
    }
  private:
    size_t getChildPos(KeyT key) const {
      // FIXME: O(N) -> O(log(N)) with binary search
      for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i] == key) {
          return i;
        }
      }
      BOOST_ASSERT(false);
    }
    void initFromBuf(const std::string &buf) {
      BOOST_ASSERT(buf.size() == sizeof(typename BTree<KeyT>::Node::_NodeFormat));
      _NodeFormat *nf = (_NodeFormat*)buf.c_str();
      this->isLeaf = nf->isLeaf;

      for (auto i = 0; i < nf->nKeyCount; ++i) {
        keys.push_back(nf->keys[i]);
        children.push_back(Location(nf->children[i].pgNo, nf->children[i].pgOff));
      }
      /**
       * For leaf node, children is actually dataLoc
       */
      if (!this->isLeaf) {
        children.push_back(Location(nf->children[nf->nKeyCount].pgNo,
                                    nf->children[nf->nKeyCount].pgOff));
      }

      if (parent) {
        auto parentLocation = parent->getLocation();
        BOOST_ASSERT(parentLocation.loc == nf->parentPgOff &&
                     (unsigned)parentLocation.pageNumber == nf->parentPgNo);
      }
    }
  private:
    BTree btree;
    std::shared_ptr<Node> parent;
    Location loc;
    std::vector<KeyT> keys;
    std::vector<Location> children;
    bool isLeaf = true;
  };
public:
  BTree(const std::string &tableName)
          : sTableName(tableName) {
    BOOST_ASSERT(BRankMin * 2 >= BRankMax);

    auto td = TableManager::getInstance()->getTableDescription(sTableName);
    BOOST_ASSERT_MSG(td != nullptr, "RecordManager::insert(), maybe you type the wrong db name.");
    this->pPager = td->getPager();
    BOOST_LOG_TRIVIAL(info) << "BTree::BTree(\"" << tableName << "\"). Constructor finished.";
  }

  ~BTree() {
  }

  void insert(KeyT key, const std::string &data) {
    auto root = getRoot();
    if (root == nullptr) {
      insertAsRoot(key, data);
    }
    else {
      insertDataFrom(root, key, data);
    }
  }
  /**
   * Insert (key, data) in the tree with the root of node.
   * @param key
   * @param node, the root of the tree
   * @param data
   * @return Returns true if insertion is a success. Returns false if key exists.
   */
  bool insertDataFrom(std::shared_ptr<Node> node, KeyT key, const std::string &data) {
    uint32_t at;
    auto targetNode = searchNode(node, key, at);
    /**
     * Insertion position could be [0, KeyCount].
     */
    BOOST_ASSERT(at <= targetNode->getKeyCount());
    BOOST_ASSERT(targetNode != nullptr);
    BOOST_ASSERT(targetNode->isLeafNode());
    if (at < targetNode->getKeyCount() && targetNode->getKey(at) == key) {
      throw KeyDuplicated(key, "");
    }
    BOOST_LOG_TRIVIAL(info) << "BTree::insertDataFrom(). targetNode.key[0] = " << targetNode->getKey(0);
    // Is a leaf node
    BOOST_ASSERT(targetNode->getKey(0) <= key);

    Location loc = RecordManager::getInstance()->insert(
            sTableName, data, /* fixed-length */false);
    BOOST_LOG_TRIVIAL(info) << "BTree::insertDataFrom(). loc = (" << loc.pageNumber << ", " << loc.loc << ")";

    targetNode->insertLocDataInLeafNode(at, key, loc);
    if (targetNode->getChildCount() == BRankMax) {
      splitLeafNode(targetNode);
    }
    else {
      targetNode->writeBack();
    }
    return true;
  }

  std::string get(KeyT key) const {
    auto ret = std::string("1");
    return ret;
  }
  void remove(KeyT key);
  void update(KeyT key, const std::string &data);
  /**
   * Traverse the btree. Read-only.
   * @param callback
   */
  void traverse(std::function<void (KeyT key, const std::string &data)> callback) const;
  /**
   * Traverse the btree.
   * @param callback: Returns true if this node is changed.
   */
  void traverse(std::function<bool (KeyT key, std::string &data)> callback);


  /**
   * Get child node i.
   * Might need 2 disk-ops
   * @param i
   * @return
   */
  std::shared_ptr<Node> getChild(std::shared_ptr<Node> parent, size_t i) {
    BOOST_ASSERT(parent != nullptr);
    Location loc = parent->getLocation();
    return parseNodeFromPage(parent, loc.pageNumber, loc.loc);
  }

  static void setupBTree(const std::string &indexName) {
    TableManager::getInstance()->buildTable(indexName, [](Pager *pPager) -> void {
      auto page0 = pPager->getPage(0);
      auto buf = page0->getBuf();

      auto p0 = (_Page0*) buf;
      p0->magic = BTREE_FILE_MAGIC_NUMBER;
      p0->nodeSize = sizeof(typename Node::_NodeFormat);
      /**
       * As a correct root pager can never be in page zero,
       *  we use 0 for representing there's no root page.
       *  That is to say, this btree is totally empty.
       */
      p0->rootNodeOff = 0;
      p0->rootNodePgNo = 0;

      page0->markDirty();
      page0->releaseBuf(buf);
    });
  }

private:
  void splitLeafNode(std::shared_ptr<Node> node) {
    node->splitNode();
  }
  void insertAsRoot(KeyT key, const std::string &data) {
    Location dataLoc = RecordManager::getInstance()->insert(
            sTableName, data, /* fixed-length */ false);
    BOOST_LOG_TRIVIAL(info) << "BTree::insertAsRoot(). key = " << key <<
                            ", data inserted at (pgNo,off) = (" <<
                            dataLoc.pageNumber << ", " << dataLoc.loc << "), " <<
                            "size = " << data.size();

    Node node = Node(*this, nullptr, key, dataLoc);
    auto rootData = node.toBuf();
    Location rootLoc = RecordManager::getInstance()->insert(
            sTableName, rootData, /* fixed-length*/ false);
    BOOST_LOG_TRIVIAL(info) << "BTree::insertAsRoot(). key = " << key <<
                            ", rootNode at (pgNo,off) = (" << rootLoc.pageNumber << ", "
                            << rootLoc.loc << ")";

    storeRootLocation(rootLoc);
  }
  void storeRootLocation(Location loc) {
    auto page0 = pPager->getPage(0);
    auto buf = page0->getBuf();
    _Page0 *p0 = (_Page0*) buf;

    BOOST_ASSERT(loc.pageNumber > 0 && loc.pageNumber < (int)pPager->getValidPageCount());
    BOOST_ASSERT(loc.loc < (int)PAGER_PAGE_SIZE);
    p0->rootNodePgNo = static_cast<uint32_t>(loc.pageNumber);
    p0->rootNodeOff = static_cast<uint16_t>(loc.loc);
    page0->markDirty();
    page0->releaseBuf(buf);
  }

  std::shared_ptr<Node> parseNodeFromPage(std::shared_ptr<Node> parent, uint32_t pgNo, uint32_t pgOff) {
    BOOST_ASSERT(pgOff < PAGER_PAGE_SIZE);
    auto record = RecordManager::getInstance()->getRecord(sTableName, Location(pgNo, pgOff));
    auto ret = std::shared_ptr<Node>(new Node(*this, parent, Location(pgNo, pgOff), record));
    return ret;
  }

  /**
   * Page 0: (all integers are little endian)
   *    +00 u32 Magic Number 0xabcd4321
   *    +04 u32 nodeSize
   *    +08 u32 rootNodePageNumber
   *    +0C u32 rootNodeOffsetInPage
   */
#pragma pack(1)
  struct _Page0 {
    uint32_t  magic;
    uint32_t  nodeSize;
    uint32_t  rootNodePgNo;
    uint32_t  rootNodeOff;
  };
#pragma pack()

  /**
   * Returns the root node.
   * @return the root node.
   */
  std::shared_ptr<Node> getRoot() {
    auto page0 = pPager->getPage(0);
    auto buf = page0->getBuf();
    _Page0 *pg0 = (_Page0*) buf;
    BOOST_ASSERT_MSG(pg0->magic == BTREE_FILE_MAGIC_NUMBER, "This file is not a btree file!");
    BOOST_ASSERT_MSG(sizeof(typename Node::_NodeFormat) == pg0->nodeSize, "NodeSize unmatched! This file might be for another index!");
    uint32_t pgNo = pg0->rootNodePgNo;
    uint32_t pgOff = pg0->rootNodeOff;
    page0->releaseBuf(buf);
    if (pgNo == 0)
      return nullptr;

    return parseNodeFromPage(nullptr, pgNo, pgOff);
  }
  /**
   * Search for the node of key `key`, returns the nearest node that targetNode.keys[0] <= key.
   * @param node
   * @param key
   * @param at: OUT. The position where the new key should be inserted.
   * @return targetNode
   */
  std::shared_ptr<Node> searchNode(std::shared_ptr<Node> node, KeyT key, uint32_t &at) {
    BOOST_ASSERT(node->getKeyCount() > 0);
    KeyT firstKey = node->getKey(0);
    KeyT lastKey = node->getKey(node->getKeyCount() - 1);
    if (node->isLeafNode()) {
      if (key < firstKey) {
        at = 0;
        return node;
      }
      else if (key >= lastKey) {
        at = node->getKeyCount();
        return node;
      }
      else {
        for (uint32_t i = 0; i < node->getKeyCount() - 1; ++i) {
          if (node->getKey(i) <= key && key < node->getKey(i + 1)) {
            at = i + 1;
            return node;
          }
        }
        BOOST_ASSERT_MSG(false, "Unreachable code reached.");
        return nullptr;
      }
    }

    BOOST_ASSERT(node->getKeyCount() + 1 == node->getChildCount());
    if (key < firstKey) {
      return searchNode(getChild(node, node->getChildCount()-1), key, at);
    }
    else if (key >= lastKey) {
      return searchNode(getChild(node, 0), key, at);
    }
    else {
      for (uint32_t i = 0; i < node->getKeyCount() - 1; ++i) {
        if (node->getKey(i) <= key && key < node->getKey(i + 1)) {
          at = i;
          return searchNode(getChild(node, i + 1), key, at);
        }
      }

      BOOST_ASSERT_MSG(false, "Unreachable code reached.");
      return nullptr;
    }
  }

private:
  std::string sTableName;
  uint32_t nNodeDataSize;
  std::shared_ptr<Pager> pPager;
};

}