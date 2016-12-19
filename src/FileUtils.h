//
// Created by alexwang on 10/13/16.
//

#ifndef TINYDBPP_FILEUTILS_H
#define TINYDBPP_FILEUTILS_H

#include <cstdint>
#include <vector>
#include "Pager/Pager.h"

namespace tinydbpp {
class FileUtils {
public:
    static int64_t fileSize(int fd);
    static Pager::PageID filePages(int fd);
    static int makeSureAtLeastFileSize(int fd, int n);
    static uint32_t readUInt32LE(const char *pBuf);
    static bool isExist(const char* whole_name);
    static int createDir(const char *sPathName);
    static int createFile(const char *str);
    static std::vector<std::string> listFiles(const char* dir);
    static bool DeleteDir(const char *dir);

    };
}


#endif //TINYDBPP_FILEUTILS_H
