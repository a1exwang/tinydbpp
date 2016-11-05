#pragma once

#include <Pager.h>
#include <map>

namespace tinydbpp {

class Pager;
class PagerPerfMon {
public:
  PagerPerfMon(const Pager *pPager) :pPager(pPager) {  }
  void incCacheHit(Pager::PageID pid) {
    auto it = cntCacheHit.find(pid);
    if (it == cntCacheHit.end()) {
      cntCacheHit[pid] = 1;
    }
    else {
      cntCacheHit[pid]++;
    }
  }

  void incReadFile(Pager::PageID pid) {
    auto it = cntCacheHit.find(pid);
    if (it == cntCacheHit.end()) {
      cntReadFile[pid] = 1;
    }
    else {
      cntReadFile[pid]++;
    }
  }

  void incWriteFile(Pager::PageID pid) {
    auto it = cntWriteFile.find(pid);
    if (it == cntWriteFile.end()) {
      cntWriteFile[pid] = 1;
    }
    else {
      cntWriteFile[pid]++;
    }
  }

  std::string report() const;
private:
  std::map<Pager::PageID, int> cntCacheHit;
  std::map<Pager::PageID, int> cntReadFile;
  std::map<Pager::PageID, int> cntWriteFile;
  const Pager *pPager;
};

}
