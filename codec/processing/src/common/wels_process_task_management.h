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
 * \file    wels_process_task_management.h
 *
 * \brief   interface for process task management
 *
 * \date    12/10/2015 Created
 *
 *************************************************************************************
 */

#ifndef _WELS_PROCESS_TASK_MANAGE_H_
#define _WELS_PROCESS_TASK_MANAGE_H_

#include "WelsLock.h"
#include "WelsThreadPool.h"
#include "WelsTask.h"
#include "typedef.h"
#include "IWelsVP.h"
#include "wels_process_task_base.h"

WELSVP_NAMESPACE_BEGIN

class IWelsProcessTaskManage {
 public:
  virtual ~IWelsProcessTaskManage() { }

  virtual EResult   Init() = 0;
  virtual void            Uninit() = 0;

  //virtual EResult   ExecuteRegional (IStrategy* pStrategy, int32_t iType, SPixMap* pSrcPixMap, SPixMap* pRefPixMap)
  //  = 0;
  virtual EResult   ExecuteTasks (IStrategy* pStrategy, int32_t iType, SPixMap* pSrcPixMap, SPixMap* pRefPixMap)
  = 0;
  
  static IWelsProcessTaskManage* CreateProcessTaskManage();

  virtual int32_t  GetThreadPoolThreadNum() = 0;
};

class CWelsProcessTaskManage : public IWelsProcessTaskManage, public WelsCommon::IWelsTaskSink {
 public:
  CWelsProcessTaskManage();
  ~CWelsProcessTaskManage();

  EResult   Init();
  
  //EResult   ExecuteRegional (IStrategy* pStrategy, int32_t iType, SPixMap* pSrcPixMap, SPixMap* pRefPixMap);
  EResult   ExecuteTasks (IStrategy* pStrategy, int32_t iType, SPixMap* pSrcPixMap, SPixMap* pRefPixMap);

  int32_t  GetThreadPoolThreadNum();

  typedef  WelsCommon::CWelsNonDuplicatedList<CWelsProcessTask>            TASKLIST_TYPE;

  //IWelsTaskSink
  virtual int OnTaskExecuted();
  virtual int OnTaskCancelled();

 protected:
  virtual EResult  CreateTasks (const int32_t kiTaskCount);

  EResult          ExecuteTaskList (TASKLIST_TYPE* pTaskList);

 protected:
  WelsCommon::CWelsThreadPool*   m_pThreadPool;

  TASKLIST_TYPE*  m_pcAllTaskList[METHOD_MASK];
  int32_t         m_iTaskNum;

  int32_t         m_iThreadNum;

  int32_t          m_iWaitTaskNum;
  WELS_EVENT       m_hTaskEvent;

  WelsCommon::CWelsLock  m_cWaitTaskNumLock;

 private:
  DISALLOW_COPY_AND_ASSIGN (CWelsProcessTaskManage);
  void  OnTaskMinusOne();

  void Uninit();
  void DestroyTasks();
  void DestroyTaskList (TASKLIST_TYPE* pTargetTaskList);

};

WELSVP_NAMESPACE_END
#endif  //header guard

