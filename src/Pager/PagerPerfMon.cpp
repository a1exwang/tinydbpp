#include <Pager/PagerPerfMon.h>
#include <sstream>
#include <json.hpp>

using namespace tinydbpp;
using json = nlohmann::json;


std::string PagerPerfMon::report() const {
  json j;
  j["filePath"] = this->pPager->getFilePath();

  j["cntCacheHit"] = json::object();
  for (auto entry : cntCacheHit) {
    std::stringstream ss;
    ss << entry.first;
    j["cntCacheHit"][ss.str()] = entry.second;
  }

  j["cntReadFile"] = json::object();
  for (auto entry : cntReadFile) {
    std::stringstream ss;
    ss << entry.first;
    j["cntReadFile"][ss.str()] = entry.second;
  }

  j["cntWriteFile"] = json::object();
  for (auto entry : cntWriteFile) {
    std::stringstream ss;
    ss << entry.first;
    j["cntWriteFile"][ss.str()] = entry.second;
  }

  return j.dump(4);
}
