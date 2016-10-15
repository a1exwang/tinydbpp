//
// Created by alexwang on 10/13/16.
//

#ifndef TINYDBPP_PAGER_H
#define TINYDBPP_PAGER_H

#include <map>
#include <string>
#include <list>
#include <memory>

namespace tinydbpp {

/**
 * Default page size 4KiB.
 * This number is a multiple of the page sizes of most file systems.
 * In addition, 4KiB is the exact size of Linux's default virtual memory page.
 */
constexpr unsigned int PAGER_PAGE_SIZE = 4096;

/**
 * 100K * 4K = 400M
 */
constexpr unsigned int PAGER_DEFAULT_MAX_PAGES = 102400;

/**
 * This exception is thrown when pager cached is full.
 */
class PagerOutOfMemory {
};

class Page;

/**
 * This class serves as a Pager manager.
 * One Pager object maps to a physical file on disk.
 *
 * Different states of a page.
 *
 *                                           releaseBuf() (if lazyMode is on)
 *                                    |--------|
 *                                    |        |
 *      new             getBuf()      |        |
 * nil  ->  Page object  ----->     Cached  <---|
 *            ^          <----      /
 *            |      releaseBuf()  /
 *             \    or cache full /
 * releaseBuf() \                / markDirty()
 *               \              /
 *                \            /
 *                 \          /
 *                  \        <
 *                    Dirty
 *
 *
 */
class Pager {
public:
  /**
   * OpenFlag for the constructor of Pager.
   */
  enum OpenFlag {
    ReadOnly, ReadWrite
  };

  typedef unsigned int PageID;
public:
  /**
   * Create a pager with file `sPath`.
   * @param sPath: The file to open.
   * @param flags: Open flags. See Pager::OpenFlag.
   * @param maxPages: Max cached pages.
   * @param lazyMode: Lazy mode means no page is freed unless cache buffer is full.
   */
  Pager(const std::string &sPath, OpenFlag flags, PageID maxPages = PAGER_DEFAULT_MAX_PAGES, bool lazyMode = false);
  ~Pager();

  /**
   * Get a page object for page `id`, starting from 0;
   * @param id: Page number.
   * @return Pointer to Page object.
   */
  std::shared_ptr<Page> getPage(PageID id);

  /**
   * When a page object needs to allocate a buffer,
   *  it calls this function to reserve a page buffer from the page cache.
   */
  void needOnePageQuota();

  /**
   * When a page object frees her buffer,
   *  it calls this function to free the reserved page buffer from the page cache.
   */
  void releaseOnePageQuota();

  /**
   * When page cache is full, it calls this function to free a "weak page".
   * A "weak page" is a page object that no one is currently using.
   */
  std::shared_ptr<Page> findVictim();

  /**
   * Destroy a page to release some space for others.
   */
  void killVictim(std::shared_ptr<Page> victim);

  int getFD() const { return this->iFd; }
  std::string getFilePath() const { return this->sFilePath; }

  /**
   * If a Page object is created, the page is called 'valid'.
   * @return page count of file.
   */
  PageID getValidPageCount() const;

  /**
   * When no one is holding a reference to the Page's buffer,
   *  the Page object calls this function to add herself to weak page list.
   */
  void addWeakPage(PageID id);

  /**
   * When cache hit happens, this function is called to remove the `weak page`.
   * Or when a Page object is destructed, this function is called to remove the `weak page`.
   * In a word, when a Page is no more a `weak page`, this function is called.
   */
  void noMoreWeakPage(PageID id);

  void writeBackAll();
private:

  std::map<PageID, std::shared_ptr<Page>> mapPages;

  /**
   * TODO change this to LRU
   */
  std::list<PageID> listWeakPages;

  std::string sFilePath;
  OpenFlag eOpenFlags;
  int iFd;
  Pager::PageID maxPages;
  Pager::PageID pagesCached;
  bool bLazyMode;
  Pager::PageID maxValidPages;
};
}


#endif //TINYDBPP_PAGER_H
