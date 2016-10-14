//
// Created by alexwang on 10/13/16.
//

#include "Page.h"
#include <FileUtils.h>
#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace tinydbpp;

Page::Page(Pager &pager, Pager::PageID id, bool lazyMode)
        : pager(pager), id(id), iBufRefCount(0), pBuf(nullptr), bDirty(false), bLazyMode(lazyMode) {
  pager.addWeakPage(id);
}

Page::~Page() {
  if (this->bDirty)
    writeBack();
  BOOST_ASSERT(this->iBufRefCount == 0);
  BOOST_LOG_TRIVIAL(info) << "Page<" << this->id << "> destructor.";
  // If pBuf != nullptr, it means this page is still a weak page.
  if (pBuf != nullptr) {
    pager.noMoreWeakPage(this->id);
    freeBuffer();
  }
}

void Page::writeBack() {
  int fd = this->pager.getFD();

  auto prevOff = lseek(fd, 0, SEEK_CUR);
  auto seekPos = lseek(fd, this->id * PAGER_PAGE_SIZE, SEEK_SET);

  BOOST_ASSERT(seekPos == this->id * PAGER_PAGE_SIZE);
  BOOST_ASSERT(this->pBuf != nullptr);

  auto byteWritten = write(fd, this->pBuf, PAGER_PAGE_SIZE);
  BOOST_ASSERT(byteWritten == PAGER_PAGE_SIZE);
  this->bDirty = false;

  /* Restore previous file offset */
  lseek(fd, prevOff, SEEK_SET);
}

int Page::decRef() {
  iBufRefCount--;
  if (iBufRefCount == 0) {
    /**
     * If lazyMode, this page becomes a weak page, and could be destroyed at any time.
     * If not lazyMode, this page is destroyed immediately.
     */
    if (bLazyMode) {
      pager.addWeakPage(this->id);
    }
    else {
      becomeVictim();
    }
  }
  return iBufRefCount;
}

char *Page::getBuf() {
  if (pBuf != nullptr) {
    // if iBufRefCount == 0, the buf is in hanging status,
    //  it could become a victim at any time.
    // if iBufRefCount > 0, the buf is actually used somewhere.
    BOOST_ASSERT(this->iBufRefCount >= 0);
    this->incRef();
    return pBuf;
  }

  pager.needOnePageQuota();

  // pBuf == nullptr
  BOOST_ASSERT(this->iBufRefCount == 0);
  int fd = this->pager.getFD();

  int fsRet = FileUtils::makeSureAtLeastFileSize(fd, PAGER_PAGE_SIZE * (id + 1));
  BOOST_ASSERT(fsRet == 0);
  this->iBufRefCount = 1;
  this->pBuf = new char[PAGER_PAGE_SIZE];
  // backup file current position
  auto prevPos = lseek(fd, 0, SEEK_CUR);

  auto curPos = lseek(fd, this->id * PAGER_PAGE_SIZE, SEEK_SET);
  BOOST_ASSERT(curPos == this->id * PAGER_PAGE_SIZE);
  auto readSize = read(fd, this->pBuf, PAGER_PAGE_SIZE);
  BOOST_LOG_TRIVIAL(info) << "Read from file<" << pager.getFilePath() << "> Page<" << this->id << "> first byte: " << (int)this->pBuf[0];
  BOOST_ASSERT(readSize == PAGER_PAGE_SIZE);

  // restore previous position
  lseek(fd, prevPos, SEEK_SET);

  return this->pBuf;
}

void Page::releaseBuf(char *pBuf) {
  BOOST_ASSERT(pBuf == this->pBuf);
  decRef();
}

int Page::incRef() {
  if (this->iBufRefCount == 0) {
    this->pager.noMoreWeakPage(this->id);
  }
  this->iBufRefCount++;
  return this->iBufRefCount;
}

void Page::becomeVictim() {
  BOOST_ASSERT(this->iBufRefCount == 0);
  BOOST_LOG_TRIVIAL(info) << "Page<" << this->id << "> has become a victim.";
  this->writeBackIfDirty();
  this->freeBuffer();
  this->pager.noMoreWeakPage(id);
}

void Page::freeBuffer() {
  BOOST_ASSERT(pBuf != nullptr);
  delete[] pBuf;
  this->pBuf = nullptr;
  this->bDirty = false;
  this->pager.releaseOnePageQuota();
}

