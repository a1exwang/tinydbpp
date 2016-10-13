//
// Created by alexwang on 10/13/16.
//

#include <Pager.h>
#include <Page.h>
#include <unistd.h>
#include <fcntl.h>
#include <boost/log/trivial.hpp>
#include <cstring>
using namespace tinydbpp;
using namespace std;

Pager::Pager(const std::string &sPath, OpenFlag flags, PageID maxPages)
  :sFilePath(sPath), eOpenFlags(flags), iFd(-1), maxPages(maxPages) {

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
  fprintf(stderr, "errno %d: msg: %s", errno, strerror(errno));
  BOOST_ASSERT(this->iFd > 0);
}
Pager::~Pager() {
  for (auto entry : this->mapPages) {
    BOOST_LOG_TRIVIAL(info) << "Unreleased page " << entry.first << " released.";
  }
  BOOST_LOG_TRIVIAL(info) << "Pager of file<" << this->sFilePath << "> destroyed.";
  close(this->iFd);
}

std::shared_ptr<Page> Pager::getPage(tinydbpp::Pager::PageID id) {
  if (this->mapPages.find(id) != this->mapPages.end()) {
    BOOST_LOG_TRIVIAL(info) << "getPage(" << id << ") cache hit";
    auto ret = this->mapPages[id];
    ret->incRef();
    return ret;
  }
  auto pPage = shared_ptr<Page>(new Page(*this, id));
  this->mapPages[id] = pPage;
  return pPage;
}


