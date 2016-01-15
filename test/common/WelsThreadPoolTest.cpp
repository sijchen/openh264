#include <gtest/gtest.h>
#include <string.h>
#include <string>
#include <list>
#include <map>

#include "typedefs.h"
#include "WelsThreadLib.h"
#include "WelsThreadPool.h"
#include "WelsTask.h"
#include "WelsThreadPoolTest.h"

#define  TEST_TASK_NUM  20

class CSimpleTask : public IWelsTask {
 public:
  static uint32_t id;

  CSimpleTask (WelsCommon::IWelsTaskSink* pSink) : IWelsTask (pSink) {
    m_uiID = id ++;
  }

  virtual ~CSimpleTask() {
  }

  virtual int32_t Execute() {
    WelsSleep (300 - m_uiID);
    //printf ("Task %d executing\n", m_uiID);
    return cmResultSuccess;
  }

 private:
  uint32_t m_uiID;
};

uint32_t CSimpleTask::id = 0;

void* OneCallingFunc (void* p) {
  CThreadPoolTest cThreadPoolTest;
  CSimpleTask* aTasks[TEST_TASK_NUM];
  CWelsThreadPool* pThreadPool = & (CWelsThreadPool::AddInstance (&cThreadPoolTest));

  int32_t  i;
  for (i = 0; i < TEST_TASK_NUM; i++) {
    aTasks[i] = new CSimpleTask (&cThreadPoolTest);
  }

  for (i = 0; i < TEST_TASK_NUM; i++) {
    pThreadPool->QueueTask (aTasks[i]);
  }

  while (cThreadPoolTest.GetTaskCount() < TEST_TASK_NUM) {
    WelsSleep (1);
  }

  for (i = 0; i < TEST_TASK_NUM; i++) {
    delete aTasks[i];
  }
  pThreadPool->RemoveInstance();

  return 0;
}


TEST (CThreadPoolTest, CThreadPoolTest) {
  OneCallingFunc (NULL);

  CWelsThreadPool* pThreadPool = & (CWelsThreadPool::AddInstance (NULL));
  EXPECT_EQ (1, pThreadPool->GetReferenceCount());
  pThreadPool->RemoveInstance();
}


TEST (CThreadPoolTest, CThreadPoolTestMulti) {
  int iCallingNum = (rand() % 10) + 1;
  WELS_THREAD_HANDLE mThreadID[iCallingNum * 3];
  int i = 0;

  for (i = 0; i < iCallingNum; i++) {
    WelsThreadCreate (& (mThreadID[i]), OneCallingFunc, NULL, 0);
    WelsSleep (1);
  }

  for (i = iCallingNum; i < iCallingNum * 2; i++) {
    WelsThreadCreate (& (mThreadID[i]), OneCallingFunc, NULL, 0);
    WelsSleep (1);
    WelsThreadJoin (mThreadID[i]);
  }

  for (i = 0; i < iCallingNum; i++) {
    WelsThreadJoin (mThreadID[i]);
  }

  for (i = iCallingNum * 2; i < iCallingNum * 3; i++) {
    WelsThreadCreate (& (mThreadID[i]), OneCallingFunc, NULL, 0);
    WelsSleep (1);
    WelsThreadJoin (mThreadID[i]);
  }

  CWelsThreadPool* pThreadPool = & (CWelsThreadPool::AddInstance (NULL));
  EXPECT_EQ (1, pThreadPool->GetReferenceCount());
  pThreadPool->RemoveInstance();
}

