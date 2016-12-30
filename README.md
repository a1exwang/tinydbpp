# tinydbpp
A simple single user, single thread, file-based SQL database implemented in C++ for *nix.

## Dependencies
  - cmake 2.8+
  - gcc 4.8+
  - boost 1.54+
  - pthread
  - QT5+

## Build

```bash
  # Download third party libraries
  git submodule init --recursive --update
  # This could be any directory
  BUILD_DIR=./build
  mkdir -p $BUILD
  cd $BUILD
  cmake $PROJECT_DIR
  make -j8
  
  # To run a test
  test/testXXX
  
  # To run Command line interface
  src/cli
  # To run graphical user interface
  gui/tinydbpp-gui/tinydbpp-gui
```

## Design
Tinydbpp is designed as multiple layers(like SQLite)
Each layer depends only on its lower layers.

Main goals for each layer.
#### Pager 
Provide a pager with which user can transparently CRUD(create/read/update/delete) a cached page.
Sample:
```c++
  Pager *pPager = new Pager("/tmp/testFile123", Pager::OpenFlag::ReadWrite);
  shared_ptr<Page> pPage = pPager->getPage(0);
  char *pBuf = pPage->getBuf();
  pBuf[0] = 'A';
  pPage->markDirty();
  pPage->releaseBuf(pBuf);
  
  pPager->
  delete pPage;
  }
    
```

#### Record
#### BTree
    
