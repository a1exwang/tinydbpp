//
// Created by alexwang on 10/13/16.
//

#include "TestPager.h"
#include "TestUtils.h"
#include <Pager.h>
#include <Page.h>

/* Must define this MACRO to prevent from linking error. */
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <memory>

using namespace tinydbpp;
using namespace std;

TestUtils utils;

TEST_CASE("Pager should write back.", "Pager") {

  auto sFilePath = utils.generateTmpFilePath();
  REQUIRE(sFilePath.size() > 0);
  Pager *pPager = new Pager(sFilePath, Pager::OpenFlag::ReadWrite);
  REQUIRE(pPager != nullptr);

  SECTION("Non-dirty page should not be saved.") {
    {
      shared_ptr<Page> pPage = pPager->getPage(0);
      char *pBuf = pPage->getBuf();
      pBuf[0] = 'A';
      pPage->releaseBuf(pBuf);
      // release page
      delete pPager;
    }
    if (utils.fileExists(sFilePath) == PAGER_PAGE_SIZE) {
      REQUIRE(utils.fileSize(sFilePath) == PAGER_PAGE_SIZE);
      REQUIRE(utils.fileCharAt(sFilePath, 0) == 0);
      remove(sFilePath.c_str());
    }
  }
  SECTION("Dirty pages should be saved when released.") {
    {
      auto pPage = pPager->getPage(1);
      char *pBuf = pPage->getBuf();
      pBuf[0] = 'A';
      pPage->markDirty();
      pPage->releaseBuf(pBuf);
      delete pPager;
    }

    REQUIRE(utils.fileExists(sFilePath));
    REQUIRE(utils.fileSize(sFilePath) >= 2 * PAGER_PAGE_SIZE);
    REQUIRE(utils.fileCharAt(sFilePath, PAGER_PAGE_SIZE) == 'A');
    remove(sFilePath.c_str());
  }
  SECTION( "Dirty page should be saved when explicitly called." ) {
    {
      auto pPage = pPager->getPage(2);
      char *pBuf = pPage->getBuf();
      pBuf[0] = 'A';
      pPage->markDirty();
      pPage->writeBack();
      pPage->releaseBuf(pBuf);
      delete pPager;
    }

    REQUIRE(utils.fileExists(sFilePath));
    REQUIRE(utils.fileSize(sFilePath) >= 3 * PAGER_PAGE_SIZE);
    REQUIRE(utils.fileCharAt(sFilePath, 2 * PAGER_PAGE_SIZE) == 'A');
    remove(sFilePath.c_str());
  }
  SECTION("All open dirty pages should be saved when pager closed.") {
    auto pPage = pPager->getPage(3);
    char *pBuf = pPage->getBuf();
    pBuf[0] = 'A';
    pPage->markDirty();
    pPage->releaseBuf(pBuf);
    // release page
    delete pPager;

    REQUIRE(utils.fileExists(sFilePath));
    REQUIRE(utils.fileSize(sFilePath) >= 4 * PAGER_PAGE_SIZE);
    REQUIRE(utils.fileCharAt(sFilePath, 3 * PAGER_PAGE_SIZE) == 'A');
    remove(sFilePath.c_str());
  }
}

TEST_CASE("Pager should manage cache", "PagerCache") {

  auto sFilePath = utils.generateTmpFilePath();
  REQUIRE(sFilePath.size() > 0);

  // the first page of file is "A\0\0....."
  char *wbuf = new char[PAGER_PAGE_SIZE * 4];
  memset(wbuf, 0, PAGER_PAGE_SIZE * 4);
  wbuf[0] = 'A';
  wbuf[PAGER_PAGE_SIZE] = 'B';
  wbuf[2 * PAGER_PAGE_SIZE] = 'C';
  wbuf[3 * PAGER_PAGE_SIZE] = 'D';
  bool writeOk = utils.fileWriteData(sFilePath, wbuf, PAGER_PAGE_SIZE * 4);
  REQUIRE(writeOk);
  delete[] wbuf;

  // Max cache size is 3 pages.
  Pager *pPager = new Pager(sFilePath, Pager::OpenFlag::ReadWrite, 3);
  REQUIRE(pPager != nullptr);

  SECTION("More pages than maxPages can be load if other page bufs are released by user.") {
    {
      auto pPage = pPager->getPage(0);
      auto buf = pPage->getBuf();
      REQUIRE(buf[0] == 'A');
      pPage->releaseBuf(buf);
      buf = nullptr;

      pPage = pPager->getPage(1);
      buf = pPage->getBuf();
      REQUIRE(buf[0] == 'B');
      pPage->releaseBuf(buf);
      buf = nullptr;

      pPage = pPager->getPage(2);
      buf = pPage->getBuf();
      REQUIRE(buf[0] == 'C');
      pPage->releaseBuf(buf);
      buf = nullptr;

      pPage = pPager->getPage(3);
      buf = pPage->getBuf();
      REQUIRE(buf[0] == 'D');
      pPage->releaseBuf(buf);
      buf = nullptr;
    }
    delete pPager;
    remove(sFilePath.c_str());
  }

}

