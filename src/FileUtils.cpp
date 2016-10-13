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
    ret = ftruncate64(fd, n);
  return ret;
}