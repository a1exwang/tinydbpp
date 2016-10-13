//
// Created by alexwang on 10/13/16.
//

#ifndef TINYDBPP_FILEUTILS_H
#define TINYDBPP_FILEUTILS_H

#include <cstdint>

namespace tinydbpp {
class FileUtils {
public:
  static int64_t fileSize(int fd);
  static int makeSureAtLeastFileSize(int fd, int n);
};
}


#endif //TINYDBPP_FILEUTILS_H
