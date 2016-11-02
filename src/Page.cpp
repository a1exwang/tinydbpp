//
// Created by alexwang on 10/13/16.
//

#include "Page.h"
#include "../test/TestUtils.h"
#include <FileUtils.h>
#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <iostream>

using namespace std;
using namespace tinydbpp;

Page::Page(Pager *pPager, Pager::PageID id, bool lazyMode)
        : pPager(pPager), id(id), iBufRefCount(0), pBuf(nullptr), bDirty(false), bLazyMode(lazyMode) {
  BOOST_ASSERT_MSG(pPager != nullptr, "Wrong parameter, pPager is nullptr.");
}

Page::~Page() {
  if (this->bDirty) {
    BOOST_ASSERT(this->pBuf != nullptr);
    BOOST_ASSERT_MSG(pPager != nullptr, "Pager destruction before Page destroyed.");
    writeBack();
  }
//  cout << "Page::~Page(). Page ID = " << this->id << endl;
  BOOST_ASSERT_MSG(this->iBufRefCount == 0, "Maybe you forget to call Page::releaseBuf().");

  // If pBuf != nullptr, it means this page is still a weak page.
  if (pBuf != nullptr) {
    if (bLazyMode && pPager != nullptr)
      pPager->noMoreWeakPage(this->id);
    freeBuffer();
  }
}

void Page::writeBack() {
  BOOST_ASSERT_MSG(pPager != nullptr, "Maybe you use the pager after deleting the pager.");
  int fd = this->pPager->getFD();

  auto prevOff = lseek(fd, 0, SEEK_CUR);
  auto seekPos = lseek(fd, this->id * PAGER_PAGE_SIZE, SEEK_SET);

  BOOST_ASSERT(seekPos == this->id * PAGER_PAGE_SIZE);
  BOOST_ASSERT(this->pBuf != nullptr);

  auto byteWritten = write(fd, this->pBuf, PAGER_PAGE_SIZE);
  cout << "Page::writeBack(). File = " << this->pPager->getFilePath()
                           << ", pageID = " << this->id << endl;
//  cout << TestUtils().hexdump(string(this->pBuf, 64));
  BOOST_ASSERT(byteWritten == PAGER_PAGE_SIZE);
  this->bDirty = false;

  /* Restore previous file offset */
  lseek(fd, prevOff, SEEK_SET);
}

int Page::decRef() {
  BOOST_ASSERT_MSG(pPager != nullptr, "Maybe you use the pager after deleting the pager.");

  iBufRefCount--;
  if (iBufRefCount == 0) {
    /**
     * If lazyMode, this page becomes a weak page, and could be destroyed at any time.
     * If not lazyMode, this page is destroyed immediately.
     */
    if (bLazyMode) {
      pPager->addWeakPage(this->id);
    }
    else {
      becomeVictim();
    }
  }
  return iBufRefCount;
}

char *Page::getBuf() {
  BOOST_ASSERT_MSG(pPager != nullptr, "Maybe you use the pager after deleting the pager.");
  BOOST_LOG_TRIVIAL(info) << "Page::getBuf() called. Page ID " << id << ". RefCount before inc " << this->iBufRefCount;
  if (pBuf != nullptr) {
    // if iBufRefCount == 0, the buf is in hanging status,
    //  it could become a victim at any time.
    // if iBufRefCount > 0, the buf is actually used somewhere.
    BOOST_ASSERT(this->iBufRefCount >= 0);
    this->incRef();
    BOOST_LOG_TRIVIAL(info) << "Page::getBuf() cache hit. Page::pBuf = " << (void*)this->pBuf;
    return pBuf;
  }

  pPager->needOnePageQuota();

  // pBuf == nullptr
  BOOST_ASSERT(this->iBufRefCount == 0);
  int fd = this->pPager->getFD();

  int fsRet = FileUtils::makeSureAtLeastFileSize(fd, PAGER_PAGE_SIZE * (id + 1));
  BOOST_ASSERT(fsRet == 0);
  this->iBufRefCount = 1;
  this->pBuf = new char[PAGER_PAGE_SIZE];
  // backup file current position
  auto prevPos = lseek(fd, 0, SEEK_CUR);

  auto curPos = lseek(fd, this->id * PAGER_PAGE_SIZE, SEEK_SET);
  BOOST_ASSERT(curPos == this->id * PAGER_PAGE_SIZE);
  auto readSize = read(fd, this->pBuf, PAGER_PAGE_SIZE);
  BOOST_LOG_TRIVIAL(info) << "Page::getBuf(). Read from file<" << pPager->getFilePath() <<
            "> Page<" << this->id << "> first byte: " << (int)this->pBuf[0];
  BOOST_ASSERT(readSize == PAGER_PAGE_SIZE);

  // restore previous position
  lseek(fd, prevPos, SEEK_SET);

  BOOST_LOG_TRIVIAL(info) << "Page::getBuf() cache miss. Read from file. Page::pBuf = " <<
            (void*)this->pBuf;
  return this->pBuf;
}

void Page::releaseBuf(char *pBuf) {
  BOOST_ASSERT(pBuf == this->pBuf);
  BOOST_LOG_TRIVIAL(info) << "Page::releaseBuf(). Page ID " << id << " RefCount before release = " << this->iBufRefCount;
  decRef();
}

int Page::incRef() {
  BOOST_ASSERT_MSG(pPager != nullptr, "Maybe you use the pager after deleting the pager.");
  if (this->iBufRefCount == 0) {
    if (bLazyMode)
      this->pPager->noMoreWeakPage(this->id);
  }
  this->iBufRefCount++;
  return this->iBufRefCount;
}

void Page::becomeVictim() {
  BOOST_ASSERT_MSG(pPager != nullptr, "Maybe you use the pager after deleting the pager.");
  BOOST_ASSERT(this->iBufRefCount == 0);
  BOOST_LOG_TRIVIAL(info) << "Page<" << this->id << "> has become a victim.";
  this->writeBackIfDirty();
  this->freeBuffer();
  if (bLazyMode)
    this->pPager->noMoreWeakPage(id);
}

void Page::freeBuffer() {
  BOOST_ASSERT(pBuf != nullptr);
  delete[] pBuf;
  this->pBuf = nullptr;
  this->bDirty = false;
  if (this->pPager != nullptr)
    this->pPager->releaseOnePageQuota();
}

void Page::pagerDied() {
//  BOOST_LOG_TRIVIAL(warning) << "Page::pagerDied()! Pager deleted before page destroyed.";
  this->pPager = nullptr;
}