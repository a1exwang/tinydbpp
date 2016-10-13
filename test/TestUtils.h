//
// Created by alexwang on 10/13/16.
//

#ifndef TINYDBPP_TESTUTILS_H
#define TINYDBPP_TESTUTILS_H

#include <string>

namespace tinydbpp {
class TestUtils {
public:
  TestUtils();
  ~TestUtils();
  std::string generateTmpFilePath() const;
  bool fileExists(const std::string &filePath) const;
  int64_t fileSize(const std::string &filePath) const;
  char fileCharAt(const std::string &filePath, uint64_t off) const;
  bool fileWriteData(const std::string &filePath, const char *data, size_t len) const;
};
}


#endif //TINYDBPP_TESTUTILS_H
