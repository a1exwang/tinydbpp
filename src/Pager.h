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
constexpr unsigned int PAGER_PAGE_SIZE = 4096;
constexpr unsigned int PAGER_DEFAULT_MAX_PAGES = 102400; // 100K * 4K = 400M

class Page;
class Pager {
public:
  enum OpenFlag {
    ReadOnly, ReadWrite
  };
  typedef unsigned int PageID;
public:
  Pager(const std::string &sPath, OpenFlag flags, PageID maxPages = PAGER_DEFAULT_MAX_PAGES);
  ~Pager();

  std::shared_ptr<Page> getPage(PageID id);

  int getFD() const { return this->iFd; }
  std::string getFilePath() const { return this->sFilePath; }
private:
  std::map<PageID, std::shared_ptr<Page>> mapPages;
  std::list<std::shared_ptr<Page>> listCachedPages;

  std::string sFilePath;
  OpenFlag eOpenFlags;
  int iFd;
  Pager::PageID maxPages;
};
}


#endif //TINYDBPP_PAGER_H
