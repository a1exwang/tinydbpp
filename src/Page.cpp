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

Page::~Page() {
  if (this->bDirty)
    writeBack();
  BOOST_ASSERT(this->iBufRefCount == 0);
  BOOST_LOG_TRIVIAL(info) << "Page<" << this->id << "> destructor.";
  if (pBuf != nullptr)
    delete[] pBuf;
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
    BOOST_LOG_TRIVIAL(info) << "Page<" << this->id << "> bufRefCount is 0, deleting...";
    if (this->bDirty) {
      this->writeBack();
    }
    delete [] pBuf;
    pBuf = nullptr;
  }
  return iBufRefCount;
}

char *Page::getBuf() {
  if (pBuf != nullptr) {
    BOOST_ASSERT(this->iBufRefCount > 0);
    this->incRef();
    return pBuf;
  }

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
  decRef();
}

int Page::incRef() { iBufRefCount++; return iBufRefCount; }