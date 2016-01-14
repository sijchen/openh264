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
 * \file    WelsThreadPool.cpp
 *
 * \brief   functions for Thread Pool
 *
 * \date    5/09/2012 Created
 *
 *************************************************************************************
 */
#include "typedefs.h"
#include "WelsThreadPool.h"

namespace WelsCommon {
  
int32_t CWelsThreadPool::m_iRefCount=0;
CWelsLock CWelsThreadPool::m_cInitLock;

CWelsThreadPool::CWelsThreadPool (IWelsThreadPoolSink* pSink, int32_t iMaxThreadNum) :
  m_pSink (pSink) {
  m_cWaitedTasks = new CWelsCircleQueue<IWelsTask>();
  m_cIdleThreads = new CWelsCircleQueue<CWelsTaskThread>();
  m_cBusyThreads = new CWelsList<CWelsTaskThread>();
  m_iMaxThreadNum = 0;

    printf("%x, %x, %x\n", m_cWaitedTasks, m_cIdleThreads, m_cBusyThreads);
    
  Init (pSink, iMaxThreadNum);
}


CWelsThreadPool::~CWelsThreadPool() {
  printf("%x, %x, %x\n", m_cWaitedTasks, m_cIdleThreads, m_cBusyThreads);
 
  m_iMaxThreadNum = 0;
  delete m_cWaitedTasks;
  delete m_cIdleThreads;
  delete m_cBusyThreads;
}

void CWelsThreadPool::Destroy() {
  Uninit();
}
  
CWelsThreadPool& CWelsThreadPool::GetInstance (IWelsThreadPoolSink* pSink, int32_t iMaxThreadNum) {
  CWelsAutoLock  cLock (m_cInitLock);
  static CWelsThreadPool m_cThreadPoolSelf(pSink, iMaxThreadNum); //To Volvet Q1: this way it cannot call the construction function, so we need to call Init as follows directly, and move all the "new" to Init?
  m_cThreadPoolSelf.Init(pSink, iMaxThreadNum);
  printf("m_iRefCount=%d, pSink=%x\n", m_iRefCount, pSink);
  ++ m_iRefCount;
  printf("m_iRefCount2=%d\n", m_iRefCount);
  return m_cThreadPoolSelf;
}

void CWelsThreadPool::RemoveInstance () {
  CWelsAutoLock  cLock (m_cInitLock);
  printf("m_iRefCount=%d\n", m_iRefCount);
  -- m_iRefCount;
  if ( 0 == m_iRefCount ) {
    Uninit();
  }
}


WELS_THREAD_ERROR_CODE CWelsThreadPool::OnTaskStart (CWelsTaskThread* pThread, IWelsTask* pTask) {
  AddThreadToBusyList (pThread);

  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::OnTaskStop (CWelsTaskThread* pThread, IWelsTask* pTask) {
  printf("CWelsThreadPool::OnTaskStop 0: Task %x at Thread %x Finished\n", pTask, pThread);

  RemoveThreadFromBusyList (pThread);
  AddThreadToIdleQueue (pThread);
  printf("CWelsThreadPool::OnTaskStop 1: Task %x at Thread %x Finished, m_pSink=%x\n", pTask, pThread, m_pSink);

  if (m_pSink) {
    m_pSink->OnTaskExecuted (pTask);
  }
  printf("CWelsThreadPool::OnTaskStop 2: Task %x at Thread %x Finished\n", pTask, pThread);

  printf("ThreadPool: Task %d at Thread %d Finished\n", (uint64_t)pTask, (uint64_t)pThread);

  SignalThread();
  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::Init (IWelsThreadPoolSink* pSink, int32_t iMaxThreadNum) {
  printf("Enter WelsThreadPool Init\n");

  
  CWelsAutoLock  cLock (m_cLockPool);
  m_pSink = pSink; //To Volvet Q2: now for one ThreadPool there is only one pSink, how can it be used for multiple modules/encoders at the same time as a singleton? Should we put a Sink into the class of IWelsTask?

  if ((NULL!=m_cWaitedTasks) && (NULL!=m_cIdleThreads) && (NULL!=m_cBusyThreads)) {
    return WELS_THREAD_ERROR_OK;
  }
  if (NULL==m_cWaitedTasks) {
    m_cWaitedTasks = new CWelsCircleQueue<IWelsTask>();}
  if (NULL==m_cIdleThreads) { m_cIdleThreads = new CWelsCircleQueue<CWelsTaskThread>();}
  if (NULL==m_cBusyThreads) { m_cBusyThreads = new CWelsList<CWelsTaskThread>();}
  m_iMaxThreadNum = 0;
  
  if (iMaxThreadNum <= 0)  iMaxThreadNum = 1;
  m_iMaxThreadNum = iMaxThreadNum;

  for (int32_t i = 0; i < m_iMaxThreadNum; i++) {
    if (WELS_THREAD_ERROR_OK != CreateIdleThread()) {
      return WELS_THREAD_ERROR_GENERAL;
    }
  }

  if (WELS_THREAD_ERROR_OK != Start()) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::Uninit() {
  CWelsAutoLock  cLock (m_cLockPool);
  
  WELS_THREAD_ERROR_CODE iReturn = WELS_THREAD_ERROR_OK;

  ClearWaitedTasks();

  while (GetBusyThreadNum() > 0) {
    //WELS_INFO_TRACE ("CWelsThreadPool::Uninit - Waiting all thread to exit");
    WelsSleep (10);
  }

  if (GetIdleThreadNum() != m_iMaxThreadNum) {
    iReturn = WELS_THREAD_ERROR_GENERAL;
  }

  m_cLockIdleTasks.Lock();
  while (m_cIdleThreads->size() > 0) {
    DestroyThread (m_cIdleThreads->begin());
    m_cIdleThreads->pop_front();
  }
  m_cLockIdleTasks.Unlock();

  m_iMaxThreadNum = 0;
  Kill();

  return iReturn;
}

void CWelsThreadPool::ExecuteTask() {
  //printf("ThreadPool: schedule tasks\n");
  CWelsTaskThread* pThread = NULL;
  IWelsTask*    pTask = NULL;
  while (GetWaitedTaskNum() > 0) {
    pThread = GetIdleThread();
    if (pThread == NULL) {
      break;
    }
    pTask = GetWaitedTask();
    //printf("ThreadPool:  ExecuteTask = %x at thread %x\n", pTask, pThread);
    pThread->SetTask (pTask);
  }
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::QueueTask (IWelsTask* pTask) {
  printf("CWelsThreadPool::QueueTask: %d, pTask=%x\n", m_iRefCount, pTask);
  
  CWelsAutoLock  cLock (m_cLockPool);

  //printf("ThreadPool:  QueueTask = %x\n", pTask);
  if (GetWaitedTaskNum() == 0) {
    CWelsTaskThread* pThread = GetIdleThread();

    if (pThread != NULL) {
      printf("ThreadPool:  ExecuteTask = %x\n", pTask);
      pThread->SetTask (pTask);

      return WELS_THREAD_ERROR_OK;
    }
  }

  AddTaskToWaitedList (pTask);

  SignalThread();
  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::CreateIdleThread() {
  CWelsTaskThread* pThread = new CWelsTaskThread (this);

  if (NULL == pThread) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  pThread->Start();
  AddThreadToIdleQueue (pThread);

  return WELS_THREAD_ERROR_OK;
}

void  CWelsThreadPool::DestroyThread (CWelsTaskThread* pThread) {
  pThread->Kill();
  delete pThread;

  return;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::AddThreadToIdleQueue (CWelsTaskThread* pThread) {
  CWelsAutoLock cLock (m_cLockIdleTasks);
  m_cIdleThreads->push_back (pThread);
  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::AddThreadToBusyList (CWelsTaskThread* pThread) {
  CWelsAutoLock cLock (m_cLockBusyTasks);
  m_cBusyThreads->push_back (pThread);
  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::RemoveThreadFromBusyList (CWelsTaskThread* pThread) {
  CWelsAutoLock cLock (m_cLockBusyTasks);
  if (m_cBusyThreads->erase(pThread)) {
      return WELS_THREAD_ERROR_OK;
  } else {
        return WELS_THREAD_ERROR_GENERAL;
  }
}

void  CWelsThreadPool::AddTaskToWaitedList (IWelsTask* pTask) {
  CWelsAutoLock  cLock (m_cLockWaitedTasks);

  m_cWaitedTasks->push_back (pTask);
  return;
}

CWelsTaskThread*   CWelsThreadPool::GetIdleThread() {
  CWelsAutoLock cLock (m_cLockIdleTasks);

  
  printf("CWelsThreadPool::GetIdleThread=%d\n", m_cIdleThreads->size());
  if (m_cIdleThreads->size() == 0) {
    return NULL;
  }

  CWelsTaskThread* pThread = m_cIdleThreads->begin();
  m_cIdleThreads->pop_front();
  return pThread;
}

int32_t  CWelsThreadPool::GetBusyThreadNum() {
  return m_cBusyThreads->size();
}

int32_t  CWelsThreadPool::GetIdleThreadNum() {
  return m_cIdleThreads->size();
}

int32_t  CWelsThreadPool::GetWaitedTaskNum() {
  return m_cWaitedTasks->size();
}

IWelsTask* CWelsThreadPool::GetWaitedTask() {
  CWelsAutoLock lock (m_cLockWaitedTasks);

  if (m_cWaitedTasks->size() == 0) {
    return NULL;
  }

  IWelsTask* pTask = m_cWaitedTasks->begin();

  m_cWaitedTasks->pop_front();

  return pTask;
}

void  CWelsThreadPool::ClearWaitedTasks() {
  CWelsAutoLock cLock (m_cLockWaitedTasks);

  if (m_pSink) {
    while (0 != m_cWaitedTasks->size()) {
      m_pSink->OnTaskCancelled (m_cWaitedTasks->begin());
      m_cWaitedTasks->pop_front();
    }
  }
}

}


