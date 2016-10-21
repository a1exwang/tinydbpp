//
// Created by alexwang on 10/13/16.
//

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
    if (utils.fileExists(sFilePath)) {
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

  /**
   * Create a file whose content is "A\0\0.....B\0\0...."
   */
  int playCount = 10;
  int maxPages = 3;
  char *wbuf = new char[PAGER_PAGE_SIZE * playCount];
  memset(wbuf, 0, PAGER_PAGE_SIZE * playCount);
  for (int i = 0; i < playCount; ++i) {
    wbuf[PAGER_PAGE_SIZE * i] = (char)('A' + i);
  }
  bool writeOk = utils.fileWriteData(sFilePath, wbuf, PAGER_PAGE_SIZE * playCount);
  REQUIRE(writeOk);
  delete[] wbuf;

  Pager *pPager = new Pager(sFilePath, Pager::OpenFlag::ReadWrite, (Pager::PageID)maxPages);
  REQUIRE(pPager != nullptr);

  SECTION("More pages than maxPages can be load if other page bufs are released by user.") {
    for (int i = 0; i < playCount; ++i) {
      auto pPage = pPager->getPage((unsigned)i);
      auto buf = pPage->getBuf();
      REQUIRE(buf[0] == (char)('A' + i));
      pPage->releaseBuf(buf);
      buf = nullptr;
    }
    delete pPager;
    remove(sFilePath.c_str());
  }
}


TEST_CASE("Pager getValidPageCount", "Pager::getValidPageCount") {
  auto sFilePath = utils.generateTmpFilePath();
  REQUIRE(sFilePath.size() > 0);

  /**
   * Create a file with 10 pages.
   */
  int playCount = 10;
  char *wbuf = new char[PAGER_PAGE_SIZE * playCount];
  memset(wbuf, 0, PAGER_PAGE_SIZE * playCount);
  bool writeOk = utils.fileWriteData(sFilePath, wbuf, PAGER_PAGE_SIZE * playCount);
  REQUIRE(writeOk);
  delete[] wbuf;

  Pager *pPager = new Pager(sFilePath, Pager::OpenFlag::ReadWrite);
  REQUIRE(pPager != nullptr);

  SECTION("ValidPageCount is correct with a file") {
    REQUIRE(pPager->getValidPageCount() == 10);
    delete pPager;
    remove(sFilePath.c_str());
  }
  SECTION("ValidPageCount is increased if new Page object is created") {
    pPager->getPage(19);
    REQUIRE(pPager->getValidPageCount() == 20);
    delete pPager;
    remove(sFilePath.c_str());
  }
  SECTION("ValidPageCount is increased if new page is written back to file") {
    auto pPage = pPager->getPage(19);
    char *buf = pPage->getBuf();
    buf[0] = 'A';
    pPage->markDirty();
    pPage->releaseBuf(buf);
    pPage->writeBackIfDirty();
    REQUIRE(pPager->getValidPageCount() == 20);
    delete pPager;
    remove(sFilePath.c_str());
  }
}

TEST_CASE("Pager should read bytes of one page", "Pager") {

  auto sFilePath = utils.generateTmpFilePath();
  REQUIRE(sFilePath.size() > 0);

  /**
   * Create a file with 1 page.
   */
  char *wbuf = new char[PAGER_PAGE_SIZE];
  for (int i = 0; i < (int)PAGER_PAGE_SIZE; ++i) {
    wbuf[i] = (uint8_t) (i % 255);
  }
  bool writeOk = utils.fileWriteData(sFilePath, wbuf, PAGER_PAGE_SIZE);
  REQUIRE(writeOk);
  delete[] wbuf;

  Pager *pPager = new Pager(sFilePath, Pager::OpenFlag::ReadWrite);
  REQUIRE(pPager != nullptr);

  SECTION("Should read bytes") {
    {
      auto pPage = pPager->getPage(0);
      auto pBuf = pPage->getBuf();
      for (int i = 0; i < (int)PAGER_PAGE_SIZE; ++i) {
        REQUIRE((uint8_t)pBuf[i] == (uint8_t)(i % 255));
      }
      pPage->releaseBuf(pBuf);
    }

    delete pPager;
  }
}