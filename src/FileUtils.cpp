//
// Created by alexwang on 10/13/16.
//


#include "FileUtils.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
using namespace tinydbpp;

int64_t FileUtils::fileSize(int fd) {
  struct stat statBuf;
  int rc = fstat(fd, &statBuf);
  return rc == 0 ? statBuf.st_size : 0;
}

int FileUtils::makeSureAtLeastFileSize(int fd, int n) {
  auto size = fileSize(fd);
  int ret = 0;
  if (size < n)
    ret = ftruncate(fd, n);
  return ret;
}

Pager::PageID FileUtils::filePages(int fd) {
  auto size = fileSize(fd);
  return (Pager::PageID) (size / PAGER_PAGE_SIZE);
}

uint32_t FileUtils::readUInt32LE(const char *pBuf) {
  return *((uint32_t*)pBuf);
}
