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

template<typename TNodeType>
class CWelsCircleQueue {
 public:
  CWelsCircleQueue() {
    m_iMaxNodeCount = 50;
    m_pCurrentQueue = static_cast<TNodeType**> (malloc (m_iMaxNodeCount * sizeof (TNodeType*)));
    //here using array to simulate list is to avoid the frequent malloc/free of Nodes which may cause fragmented memory
    m_iCurrentListStart = m_iCurrentListEnd = 0;
  };
  ~CWelsCircleQueue() {
    free (m_pCurrentQueue);
  };

  int32_t size() {
    return ((m_iCurrentListEnd >= m_iCurrentListStart)
            ? (m_iCurrentListEnd - m_iCurrentListStart)
            : (m_iMaxNodeCount - m_iCurrentListStart + m_iCurrentListEnd));
  }

  int32_t push_back (TNodeType* pNode) {
    if ((NULL != pNode) && (find (pNode))) {      //not checking NULL for easier testing
      return 1;
    }
    int32_t ret = InternalPushBack (pNode);
    return ret;
  }

  bool find (TNodeType* pNode) {
    if (size() > 0) {
      if (m_iCurrentListEnd > m_iCurrentListStart) {
        for (int32_t idx = m_iCurrentListStart; idx < m_iCurrentListEnd; idx++) {
          if (pNode == m_pCurrentQueue[idx]) {
            return true;
          }
        }
      } else {
        for (int32_t idx = m_iCurrentListStart; idx < m_iMaxNodeCount; idx++) {
          if (pNode == m_pCurrentQueue[idx]) {
            return true;
          }
        }
        for (int32_t idx = 0; idx < m_iCurrentListEnd; idx++) {
          if (pNode == m_pCurrentQueue[idx]) {
            return true;
          }
        }

      }
    }
    return false;
  }

  void pop_front() {
    if (size() > 0) {
      m_pCurrentQueue[m_iCurrentListStart] = NULL;
      m_iCurrentListStart = ((m_iCurrentListStart < (m_iMaxNodeCount - 1))
                             ? (m_iCurrentListStart + 1)
                             : 0);
    }
  }

  TNodeType* begin() {
    return m_pCurrentQueue[m_iCurrentListStart];
  }
 private:
  int32_t InternalPushBack (TNodeType* pNode) {
    m_pCurrentQueue[m_iCurrentListEnd] = pNode;
    m_iCurrentListEnd ++;

    if (m_iCurrentListEnd == m_iMaxNodeCount) {
      m_iCurrentListEnd = 0;
    }
    if (m_iCurrentListEnd == m_iCurrentListStart) {
      int32_t ret = ExpandList();
      if (ret) {
        return 1;
      }
    }
    return 0;
  }

  int32_t ExpandList() {
    TNodeType** tmpCurrentTaskQueue = static_cast<TNodeType**> (malloc (m_iMaxNodeCount * 2 * sizeof (TNodeType*)));
    if (tmpCurrentTaskQueue == NULL) {
      return 1;
    }

    memcpy (tmpCurrentTaskQueue,
            (m_pCurrentQueue + m_iCurrentListStart),
            (m_iMaxNodeCount - m_iCurrentListStart)*sizeof (TNodeType*));
    if (m_iCurrentListEnd > 0) {
      memcpy (tmpCurrentTaskQueue + m_iMaxNodeCount - m_iCurrentListStart,
              m_pCurrentQueue,
              m_iCurrentListEnd * sizeof (TNodeType*));
    }

    free (m_pCurrentQueue);

    m_pCurrentQueue = tmpCurrentTaskQueue;
    m_iCurrentListEnd = m_iMaxNodeCount;
    m_iCurrentListStart = 0;
    m_iMaxNodeCount = m_iMaxNodeCount * 2;

    return 0;
  }
  int32_t m_iCurrentListStart;
  int32_t m_iCurrentListEnd;
  int32_t m_iMaxNodeCount;
  TNodeType** m_pCurrentQueue;
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
  CWelsCircleQueue<IWelsTask>* m_cWaitedTasks;
  //std::map<uintptr_t, CWelsTaskThread*>  m_cIdleThreads;
  CWelsCircleQueue<CWelsTaskThread>* m_cIdleThreads;
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



