//
// Created by alexwang on 10/13/16.
//

#include <Pager/Pager.h>
#include <Pager/Page.h>
#include <unistd.h>
#include <fcntl.h>
#include <boost/log/trivial.hpp>
#include <cstring>
#include "FileUtils.h"
#include <Pager/PagerPerfMon.h>

using namespace tinydbpp;
using namespace std;

Pager::Pager(const std::string &sPath, OpenFlag flags, PageID maxPages, bool lazyMode)
  :sFilePath(sPath), eOpenFlags(flags), iFd(-1), maxPages(maxPages), pagesCached(0), bLazyMode(lazyMode) {

  int oFlags = O_CREAT;
  switch(flags) {
  case ReadOnly:
    oFlags |= O_RDONLY;
    break;
  case ReadWrite:
    oFlags |= O_RDWR;
    break;
  default:
    BOOST_LOG_TRIVIAL(error) << "Wrong argument `flags`";
    exit(1);
  }
  this->iFd = open(sPath.c_str(), oFlags, 0700);
  if (this->iFd < 0) {
    BOOST_LOG_TRIVIAL(warning)
      << "Pager::Pager(" << sPath << "), open file errno: " <<
      errno << ", message: " << strerror(errno);
  }
  BOOST_ASSERT(this->iFd > 0);

  auto fileSize = FileUtils::fileSize(this->iFd);
  BOOST_ASSERT(fileSize >= 0 && fileSize % PAGER_PAGE_SIZE == 0);
  this->maxValidPages = (Pager::PageID) (fileSize / PAGER_PAGE_SIZE);

  this->pPerfMon = new PagerPerfMon(this);
}

Pager::~Pager() {
  __isDestructing = true;
  writeBackAll();
  close(this->iFd);

  /**
   * NOTE: DO NOT use Boost.Log in static variable destructor.
   */
//  BOOST_LOG_TRIVIAL(info) << "Pager of file<" << this->sFilePath << "> destroyed.";
  for (auto entry : this->mapPages) {
    // BOOST_LOG_TRIVIAL(info) << "Unreleased page " << entry.first << " released.";
    entry.second->pagerDied();
  }
  if (this->pPerfMon) {
    delete pPerfMon;
  }
}

std::shared_ptr<Page> Pager::getPage(tinydbpp::Pager::PageID id) {
  if (this->mapPages.find(id) != this->mapPages.end()) {
    BOOST_LOG_TRIVIAL(info) << "Pager::getPage(" << id << ") cache hit";
    auto ret = this->mapPages[id];
    return ret;
  }
  FileUtils::makeSureAtLeastFileSize(this->iFd, (id + 1) * PAGER_PAGE_SIZE );
  this->maxValidPages = FileUtils::filePages(this->iFd);
  auto pPage = shared_ptr<Page>(new Page(this, id, this->bLazyMode));
  this->mapPages[id] = pPage;
  return pPage;
}

void Pager::needOnePageQuota() {
  if (this->pagesCached >= this->maxPages) {
    shared_ptr<Page> victim = findVictim();
    if (victim == nullptr) {
      throw PagerOutOfMemory();
    }
    killVictim(victim);
  }

  this->pagesCached++;
}

void Pager::killVictim(shared_ptr<Page> victim) {
  Pager::PageID id = victim->getID();
  auto p = this->mapPages[id];
  BOOST_ASSERT(p == victim);
  victim->becomeVictim();
}

void Pager::releaseOnePageQuota() {
  pagesCached--;
}

std::shared_ptr<Page> Pager::findVictim() {
  BOOST_ASSERT(this->listWeakPages.size() > 0);
  PageID id = *this->listWeakPages.begin();
  BOOST_ASSERT(this->mapPages.find(id) != this->mapPages.end());
  return this->mapPages[id];
}

void Pager::noMoreWeakPage(PageID id) {
  // TODO O(N) -> O(1)
  auto it = std::find(listWeakPages.begin(), listWeakPages.end(), id);
//  BOOST_LOG_TRIVIAL(info) << "Pager::noMoreWeakPage(), Page " << id << " is no more a weak page.";
  BOOST_ASSERT(it != listWeakPages.end());
  listWeakPages.erase(it);
}

void Pager::addWeakPage(PageID id) {
  listWeakPages.push_back(id);
}

void Pager::writeBackAll() {
  for (auto entry : this->mapPages) {
    entry.second->writeBackIfDirty();
  }
}

Pager::PageID Pager::getValidPageCount() const {
  return this->maxValidPages;
}

