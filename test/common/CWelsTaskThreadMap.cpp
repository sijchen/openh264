#include <gtest/gtest.h>
#include "WelsThreadPool.h"

using namespace WelsCommon;

TEST (CWelsTaskThreadMap, CWelsTaskThreadMapOne) {
  const int32_t kiMaxThreadNum = (rand() % 63) + 1;
  CWelsTaskThreadMap* cTaskMap = new CWelsTaskThreadMap (kiMaxThreadNum);
  EXPECT_TRUE (NULL != cTaskMap);
  CWelsTaskThread* pTaskThread = new CWelsTaskThread(NULL);
  EXPECT_TRUE (NULL != pTaskThread);

  uintptr_t id = pTaskThread->GetID();

  EXPECT_TRUE (0 == cTaskMap->size());
  for (int i = 0; i < kiMaxThreadNum * 2; i++) {
    cTaskMap->push_back (id, pTaskThread);
    EXPECT_TRUE (1 == cTaskMap->size()) << "after push size=" << cTaskMap->size() ;

    cTaskMap->pop_front();
    EXPECT_TRUE (0 == cTaskMap->size()) << "after pop size=" << cTaskMap->size() ;
  }

  for (int i = 0; i < kiMaxThreadNum; i++) {
    cTaskMap->push_back (id, pTaskThread);
    EXPECT_TRUE (1 == cTaskMap->size()) << "after push size=" << cTaskMap->size() ;

    cTaskMap->pop_front();
    cTaskMap->pop_front();
    EXPECT_TRUE (0 == cTaskMap->size()) << "after pop size=" << cTaskMap->size() ;
  }

  delete cTaskMap;
  delete pTaskThread;
}

TEST (CWelsTaskThreadMap, CWelsTaskThreadMap) {
  const int32_t kiMaxThreadNum = 2;
  CWelsTaskThreadMap* cTaskMap = new CWelsTaskThreadMap (kiMaxThreadNum);
  EXPECT_TRUE (NULL != cTaskMap);
  CWelsTaskThread* pTaskThread1 = new CWelsTaskThread(NULL);
  EXPECT_TRUE (NULL != pTaskThread1);
  CWelsTaskThread* pTaskThread2 = new CWelsTaskThread(NULL);
  EXPECT_TRUE (NULL != pTaskThread2);
  CWelsTaskThread* pTaskThread3 = new CWelsTaskThread(NULL);
  EXPECT_TRUE (NULL != pTaskThread3);

  uintptr_t id1 = pTaskThread1->GetID();
  uintptr_t id2 = pTaskThread2->GetID();
  uintptr_t id3 = pTaskThread3->GetID();

  EXPECT_TRUE (cTaskMap->push_back (id1, pTaskThread1));
  EXPECT_TRUE (cTaskMap->push_back (id2, pTaskThread2));
  EXPECT_FALSE (cTaskMap->push_back (id3, pTaskThread3));
  EXPECT_TRUE (2 == cTaskMap->size());

  EXPECT_TRUE (pTaskThread1 == cTaskMap->find (id1));
  EXPECT_TRUE (pTaskThread2 == cTaskMap->find (id2));

  cTaskMap->erase (id1);
  EXPECT_TRUE (1 == cTaskMap->size());

  EXPECT_TRUE (cTaskMap->push_back (id3, pTaskThread3));
  EXPECT_TRUE (2 == cTaskMap->size());

  cTaskMap->erase (id2);
  EXPECT_TRUE (1 == cTaskMap->size());

  EXPECT_TRUE (cTaskMap->push_back (id1, pTaskThread1));
  EXPECT_TRUE (2 == cTaskMap->size());

  EXPECT_FALSE (cTaskMap->push_back (id3, pTaskThread3));
  EXPECT_TRUE (2 == cTaskMap->size());

  while (NULL != cTaskMap->begin()) {
    cTaskMap->pop_front();
  }
  EXPECT_TRUE (0 == cTaskMap->size());
  
  delete cTaskMap;
  delete pTaskThread1;
  delete pTaskThread2;
  delete pTaskThread3;
}
