/*!
 * \copy
 *     Copyright (c)  2009-2015, Cisco Systems
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions
 *     are met:
 *
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in
 *          the documentation and/or other materials provided with the
 *          distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *     LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *     ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *     POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * \file    WelsThreadPool.h
 *
 * \brief   Interfaces introduced in thread pool
 *
 * \date    5/09/2012 Created
 *
 *************************************************************************************
 */


#ifndef _WELS_THREAD_POOL_H_
#define _WELS_THREAD_POOL_H_

#include <map>
#include <stdio.h>
#include "WelsTask.h"
#include "WelsTaskThread.h"

namespace WelsCommon {

class IWelsThreadPoolSink {
 public:
  virtual WELS_THREAD_ERROR_CODE OnTaskExecuted (IWelsTask* pTask) = 0;
  virtual WELS_THREAD_ERROR_CODE OnTaskCancelled (IWelsTask* pTask) = 0;
};


class CWelsTaskList {
 public:
  CWelsTaskList() {
    m_iMaxTaskCount = 50;
    m_pCurrentTaskQueue = static_cast<IWelsTask**> (malloc (m_iMaxTaskCount * sizeof (IWelsTask*)));
    m_iCurrentTaskStart = m_iCurrentTaskEnd = 0;
  };
  ~CWelsTaskList() {
    free (m_pCurrentTaskQueue);
  };

  int32_t size() {
    return ((m_iCurrentTaskEnd >= m_iCurrentTaskStart)
            ? (m_iCurrentTaskEnd - m_iCurrentTaskStart)
            : (m_iMaxTaskCount - m_iCurrentTaskStart + m_iCurrentTaskEnd));
  }

  int32_t push_back (IWelsTask* pTask) {
    m_pCurrentTaskQueue[m_iCurrentTaskEnd] = pTask;
    m_iCurrentTaskEnd ++;

    if (m_iCurrentTaskEnd == m_iMaxTaskCount) {
      m_iCurrentTaskEnd = 0;
    }
    if (m_iCurrentTaskEnd == m_iCurrentTaskStart) {
      int32_t ret = ExpandList();
      if (ret) {
        return 1;
      }
    }
    return 0;
  }

  void pop_front() {
    if (size() > 0) {
      m_pCurrentTaskQueue[m_iCurrentTaskStart] = NULL;
      m_iCurrentTaskStart = ((m_iCurrentTaskStart < (m_iMaxTaskCount - 1))
                             ? (m_iCurrentTaskStart + 1)
                             : 0);
    }
  }

  IWelsTask* begin() {
    return m_pCurrentTaskQueue[m_iCurrentTaskStart];
  }
 private:
  int32_t ExpandList() {
    IWelsTask** tmpCurrentTaskQueue = static_cast<IWelsTask**> (malloc (m_iMaxTaskCount * 2 * sizeof (IWelsTask*)));
    if (tmpCurrentTaskQueue == NULL) {
      return 1;
    }

    memcpy (tmpCurrentTaskQueue,
            (m_pCurrentTaskQueue + m_iCurrentTaskStart),
            (m_iMaxTaskCount - m_iCurrentTaskStart)*sizeof (IWelsTask*));
    if (m_iCurrentTaskEnd > 0) {
      memcpy (tmpCurrentTaskQueue + m_iMaxTaskCount - m_iCurrentTaskStart,
              m_pCurrentTaskQueue,
              m_iCurrentTaskEnd * sizeof (IWelsTask*));
    }

    free (m_pCurrentTaskQueue);

    m_pCurrentTaskQueue = tmpCurrentTaskQueue;
    m_iCurrentTaskEnd = m_iMaxTaskCount;
    m_iCurrentTaskStart = 0;
    m_iMaxTaskCount = m_iMaxTaskCount * 2;

    return 0;
  }
  int32_t m_iCurrentTaskStart;
  int32_t m_iCurrentTaskEnd;
  int32_t m_iMaxTaskCount;
  IWelsTask** m_pCurrentTaskQueue;
};

class CWelsTaskThreadMap {
 public:
  CWelsTaskThreadMap (int32_t iMaxThreadNum) {
    m_iMaxTaskThreadCount = iMaxThreadNum;
    m_pCurrentTaskThreadIdQueue = static_cast<uintptr_t*> (malloc (iMaxThreadNum * sizeof (uintptr_t)));
    m_pCurrentTaskThreadQueue = static_cast<CWelsTaskThread**> (malloc (iMaxThreadNum * sizeof (CWelsTaskThread*)));
    for (int32_t i = 0; i < m_iMaxTaskThreadCount; i++) {
      m_pCurrentTaskThreadIdQueue[i] = 0;
      m_pCurrentTaskThreadQueue[i] = NULL;
    }
  };
  ~CWelsTaskThreadMap() {
    free (m_pCurrentTaskThreadIdQueue);
    free (m_pCurrentTaskThreadQueue);
  };

  int32_t size() {
    int32_t iCount = 0;
    for (int32_t idx = 0; idx < m_iMaxTaskThreadCount; idx++) {
      if (NULL != m_pCurrentTaskThreadQueue[idx]) {
        iCount ++;
      }
    }
    return iCount;
  }

  void erase (uintptr_t uiCurId) {
    for (int32_t idx = 0; idx < m_iMaxTaskThreadCount; idx++) {
      if ((uiCurId == m_pCurrentTaskThreadIdQueue[idx]) && (NULL != m_pCurrentTaskThreadQueue[idx])) {
        m_pCurrentTaskThreadIdQueue[idx] = 0;
        m_pCurrentTaskThreadQueue[idx] = NULL;
        return;
      }
    }
  }

  CWelsTaskThread* begin() {
    for (int32_t idx = 0; idx < m_iMaxTaskThreadCount; idx++) {
      if (NULL != m_pCurrentTaskThreadQueue[idx]) {
        return m_pCurrentTaskThreadQueue[idx];
      }
    }
    return NULL;
  }

  void pop_front() {
    for (int32_t idx = 0; idx < m_iMaxTaskThreadCount; idx++) {
      if (NULL != m_pCurrentTaskThreadQueue[idx]) {
        m_pCurrentTaskThreadIdQueue[idx] = 0;
        m_pCurrentTaskThreadQueue[idx] = NULL;
        return;
      }
    }
  }

  CWelsTaskThread* find (uintptr_t uiCurId) {
    for (int32_t idx = 0; idx < m_iMaxTaskThreadCount; idx++) {
      if ((uiCurId == m_pCurrentTaskThreadIdQueue[idx]) && (NULL != m_pCurrentTaskThreadQueue[idx])) {
        return m_pCurrentTaskThreadQueue[idx];
      }
    }
    return NULL;
  }

  bool push_back (uintptr_t uiCurId, CWelsTaskThread* pThread) {
    CWelsTaskThread* pointer = find (uiCurId);
    if (NULL != pointer) {
      return false;
    }

    for (int32_t i = 0; i < m_iMaxTaskThreadCount; i++) {
      if (NULL == m_pCurrentTaskThreadQueue[i]) {
        m_pCurrentTaskThreadIdQueue[i] = uiCurId;
        m_pCurrentTaskThreadQueue[i] = pThread;
        return true;
      }
    }
    return false;
  }

 private:
  int32_t m_iMaxTaskThreadCount;

  uintptr_t* m_pCurrentTaskThreadIdQueue;
  CWelsTaskThread** m_pCurrentTaskThreadQueue;
};


class  CWelsThreadPool : public CWelsThread, public IWelsTaskThreadSink {
 public:
  enum {
    DEFAULT_THREAD_NUM = 4,
  };

  CWelsThreadPool (IWelsThreadPoolSink* pSink = NULL, int32_t iMaxThreadNum = DEFAULT_THREAD_NUM);
  virtual ~CWelsThreadPool();

  //IWelsTaskThreadSink
  virtual WELS_THREAD_ERROR_CODE OnTaskStart (CWelsTaskThread* pThread,  IWelsTask* pTask);
  virtual WELS_THREAD_ERROR_CODE OnTaskStop (CWelsTaskThread* pThread,  IWelsTask* pTask);

  //  CWelsThread
  virtual void ExecuteTask();

  WELS_THREAD_ERROR_CODE  QueueTask (IWelsTask* pTask);
  int32_t        GetThreadNum() const {
    return m_iMaxThreadNum;
  }

 protected:
  WELS_THREAD_ERROR_CODE Init (int32_t iMaxThreadNum = DEFAULT_THREAD_NUM);
  WELS_THREAD_ERROR_CODE Uninit();

  WELS_THREAD_ERROR_CODE CreateIdleThread();
  void           DestroyThread (CWelsTaskThread* pThread);
  WELS_THREAD_ERROR_CODE AddThreadToIdleMap (CWelsTaskThread* pThread);
  WELS_THREAD_ERROR_CODE AddThreadToBusyMap (CWelsTaskThread* pThread);
  WELS_THREAD_ERROR_CODE RemoveThreadFromBusyMap (CWelsTaskThread* pThread);
  void           AddTaskToWaitedList (IWelsTask* pTask);
  CWelsTaskThread*   GetIdleThread();
  IWelsTask*         GetWaitedTask();
  int32_t            GetIdleThreadNum();
  int32_t            GetBusyThreadNum();
  int32_t            GetWaitedTaskNum();
  void               ClearWaitedTasks();

 private:
  int32_t   m_iMaxThreadNum;
  //std::list<IWelsTask*>    m_cWaitedTasks;
  CWelsTaskList* m_cWaitedTasks;
  std::map<uintptr_t, CWelsTaskThread*>  m_cIdleThreads;
  std::map<uintptr_t, CWelsTaskThread*>  m_cBusyThreads;
  IWelsThreadPoolSink*   m_pSink;

  CWelsLock   m_cLockPool;
  CWelsLock   m_cLockWaitedTasks;
  CWelsLock   m_cLockIdleTasks;
  CWelsLock   m_cLockBusyTasks;

  DISALLOW_COPY_AND_ASSIGN (CWelsThreadPool);
};

}


#endif



