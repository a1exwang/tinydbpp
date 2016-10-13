//
// Created by alexwang on 10/13/16.
//

#include "TestUtils.h"
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <fcntl.h>
#include <sys/types.h>
#include <error.h>
#include <cstring>

using namespace tinydbpp;
using namespace std;
std::string TestUtils::generateTmpFilePath() const {
  stringstream ss;
  string dir = "/tmp/testTmpPath65336";
  int status = mkdir(dir.c_str(), 0700);
  BOOST_ASSERT(status == 0 || errno == EEXIST);

  int randInt = rand();
  ss << dir << "/file_" << randInt;
  return ss.str();
}

bool TestUtils::fileExists(const std::string &filePath) const {
  return (access(filePath.c_str(), F_OK) != -1);
}

int64_t TestUtils::fileSize(const std::string &filePath) const {
  struct stat statBuf;
  int rc = stat(filePath.c_str(), &statBuf);
  return rc == 0 ? statBuf.st_size : 0;
}

char TestUtils::fileCharAt(const std::string &filePath, uint64_t off) const {
  int fd = open(filePath.c_str(), O_RDONLY);
  if (fd < 0)
    return 0;

  lseek(fd, off, SEEK_SET);
  char c;
  ssize_t n = read(fd, &c, 1);
  if (n == 0) {
    close(fd);
    return 0;
  }
  close(fd);
  return c;
}

bool TestUtils::fileWriteData(const std::string &filePath, const char *data, size_t len) const {
  int fd = open(filePath.c_str(), O_RDWR | O_CREAT, 0700);
  if (fd <= 0)
    return false;
  auto pos = lseek(fd, 0, SEEK_SET);
  BOOST_ASSERT(pos == 0);
  auto bytesWritten = write(fd, data, len);
  BOOST_ASSERT(bytesWritten >= 0 && (size_t)bytesWritten == len);
  auto status = close(fd);
  BOOST_ASSERT(status == 0);

  BOOST_LOG_TRIVIAL(info) << "file is at " << filePath;

  return true;
}

TestUtils::TestUtils() {
  /**
   * TODO: Currently, seed is set to 0 to make results deterministic.
   * Should change to
   *  srand(time(nullptr));
   */
  srand(time(nullptr));
}

TestUtils::~TestUtils() {

}

