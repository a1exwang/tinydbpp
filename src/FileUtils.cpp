//
// Created by alexwang on 10/13/16.
//


#include "FileUtils.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <boost/range/iterator_range_core.hpp>

using namespace tinydbpp;
using namespace boost::filesystem;
using namespace std;
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

bool FileUtils::isExist(const char *whole_name) {
    FILE* testFile = fopen(whole_name, "r");
    if(testFile == NULL) return false;
    fclose(testFile);
    return true;
}

int FileUtils::createDir(const char *sPathName) {
  char DirName[256];
  strcpy(DirName, sPathName);
  size_t len = strlen(DirName);
  if(DirName[len - 1] != '/')
    strcat(DirName,   "/");
  len = strlen(DirName);
  for(int i = 1; i < len; i++)
  {
      if(DirName[i]=='/')
      {
          DirName[i] = 0;
          if(access(DirName, NULL) != 0)
          {
              if(mkdir(DirName, 0755)==-1)
              {
                  perror("mkdir error");
                  return -1;
              }
          }
          DirName[i] = '/';
      }
  }
  return 0;
}

int FileUtils::createFile(const char *str) {
    size_t len = strlen(str);
    int pos_of_dir = (int)(len - 1);
    for(;pos_of_dir >= 0 && str[pos_of_dir] != '/';pos_of_dir --);
    if(pos_of_dir > 0)
    {
        char DirName[256];
        memcpy(DirName, str, (size_t)pos_of_dir);
        DirName[pos_of_dir] = '\0';
        createDir(DirName);
    }
    FILE* res = fopen(str, "w");
    fclose(res);
    if(res == NULL) return -1;
    else return 0;
}

std::vector<std::string> FileUtils::listFiles(const char *dir) {
    std::vector<std::string> ret;
    for(auto& entry : boost::make_iterator_range(directory_iterator(path(dir)), {})) {
        string tmp = entry.path().filename().string();
        if (tmp[0] != '.')
            ret.push_back(tmp);
    }
    return ret;
}
