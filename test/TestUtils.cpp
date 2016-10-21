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
#include <cstring>
#include <sstream>
#include <cmath>
#include <iomanip>

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

std::string TestUtils::hexdump(const char *ptr, size_t count) const {
  stringstream ss;
  size_t items_per_line = 16;
  size_t line_count = (size_t) ceil((float)count / items_per_line);
  for (size_t line_number = 0; line_number < line_count; ++line_number) {
    ss << "+" << setbase(16) << setfill('0') << setw(2) << line_number * items_per_line << "  ";
    for (size_t col_number = 0; col_number < items_per_line; ++col_number) {
      ss << setfill('0') << setw(2) << (unsigned int)(uint8_t)ptr[line_number * items_per_line + col_number] << ' ';
    }
    ss << " |>";
    for (size_t col_number = 0; col_number < items_per_line; ++col_number) {
      char c = ptr[line_number * items_per_line + col_number];
      switch(c) {
      case '\n':
        ss << "\\n"; break;
      case '\r':
        ss << "\\r"; break;
      default:
        ss << c; break;
      }
    }
    ss << endl;
  }
  return ss.str();
}

std::string TestUtils::hexdump(std::string str) const {
  return hexdump(str.c_str(), str.size());
}

