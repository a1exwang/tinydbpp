//
// Created by alexwang on 10/13/16.
//

#ifndef TINYDBPP_FILEUTILS_H
#define TINYDBPP_FILEUTILS_H

#include <cstdint>
#include "Pager.h"

namespace tinydbpp {
class FileUtils {
public:
  static int64_t fileSize(int fd);
  static Pager::PageID filePages(int fd);
  static int makeSureAtLeastFileSize(int fd, int n);
  static uint32_t readUInt32LE(const char *pBuf);
};
}


#endif //TINYDBPP_FILEUTILS_H
