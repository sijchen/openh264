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
#include "WelsCircleQueue.h"

namespace WelsCommon {

class IWelsThreadPoolSink {
 public:
  virtual WELS_THREAD_ERROR_CODE OnTaskExecuted (IWelsTask* pTask) = 0;
  virtual WELS_THREAD_ERROR_CODE OnTaskCancelled (IWelsTask* pTask) = 0;
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



