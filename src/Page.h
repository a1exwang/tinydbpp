//
// Created by alexwang on 10/13/16.
//

#ifndef TINYDBPP_PAGE_H
#define TINYDBPP_PAGE_H
#include <Pager.h>

namespace tinydbpp {
class Page {
public:

  /**
   * Constructor. Only Pager object should call this method.
   */
  Page(Pager &pager, Pager::PageID id) :pager(pager), id(id), iBufRefCount(0), pBuf(nullptr), bDirty(false) { }

  /**
   * Destructor.
   * If the page is dirty, it automatically writes back.
   */
  ~Page();

  /**
   * Get the content of the page.
   * The buffer size is PAGER_PAGE_SIZE.
   * User must call release(pBuf) after using it.
   * @return: The buffer.
   */
  char* getBuf();

  /**
   * Release the buffer that previous `getBuf` call returns.
   */
  void releaseBuf(char *pBuf);

  /**
   * Increase page buffer reference count.
   * @return: new reference count.
   */
  int incRef();

  /**
   * Decrease page buffer reference count.
   * If reference count is zero, delete page buffer.
   * @return: new reference count.
   */
  int decRef();

  /**
   * Mark this page as `dirty`.
   * It means the content of this page has changed, and
   *  should be saved to file.
   */
  void markDirty() {
    this->bDirty = true;
  }

  /**
   * Write the buffer to file.
   * And bDirty is set to false.
   */
  void writeBack();
private:
  Pager &pager;
  Pager::PageID id;
  int iBufRefCount;
  char *pBuf;
  bool bDirty;
};
}


#endif //TINYDBPP_PAGE_H
