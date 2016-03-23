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
 * \file    wels_process_task_management.cpp
 *
 * \brief   function for task process management
 *
 * \date    12/10/2015 Created
 *
 *************************************************************************************
 */
#include <string.h>
#include <assert.h>

#include "typedefs.h"
#include "utils.h"
#include "WelsLock.h"
#include "memory_align.h"

#include "wels_process_task_management.h"

#include "../denoise/denoise.h"
#include "../downsample/downsample.h"
#include "../scrolldetection/ScrollDetection.h"
#include "../scenechangedetection/wels_task_preprocess.h"
#include "../vaacalc/vaacalculation.h"
#include "../backgrounddetection/BackgroundDetection.h"
#include "../adaptivequantization/AdaptiveQuantization.h"
#include "../complexityanalysis/ComplexityAnalysis.h"
#include "../imagerotate/imagerotate.h"

WELSVP_NAMESPACE_BEGIN



CWelsProcessTaskManage::CWelsProcessTaskManage()
: m_pThreadPool (NULL),
m_iWaitTaskNum (0) {
  m_iTaskNum = 0;
  WelsEventOpen (&m_hTaskEvent);
}

CWelsProcessTaskManage::~CWelsProcessTaskManage() {
  //fprintf(stdout, "~CWelsProcessTaskManage\n");
  Uninit();
}

EResult CWelsProcessTaskManage::Init () {
  //fprintf(stdout, "m_pThreadPool = &(CWelsThreadPool::GetInstance, this=%x\n", this);

  m_pThreadPool = & (WelsCommon::CWelsThreadPool::AddReference ());
  WELS_VERIFY_RETURN_IF (RET_OUTOFMEMORY, NULL == m_pThreadPool)
  //fprintf(stdout, "m_pThreadPool = &(CWelsThreadPool::GetInstance3\n");
  
  return CreateTasks (m_pThreadPool->GetThreadNum ());
}

void   CWelsProcessTaskManage::Uninit() {
  DestroyTasks();
  //fprintf(stdout, "m_pThreadPool = m_pThreadPool->RemoveInstance\n");
  m_pThreadPool->RemoveInstance();
  //WELS_DELETE_OP (m_pThreadPool);
  
  //fprintf(stdout, "m_pThreadPool = m_pThreadPool->RemoveInstance2\n");

  WelsEventClose (&m_hTaskEvent);
}

EResult CWelsProcessTaskManage::CreateTasks (const int32_t kiTaskCount) {
  CWelsProcessTask* pTask = NULL;

  for (int idx = 0; idx < kiTaskCount; idx++) {
    pTask = WELS_NEW_OP (CWelsSceneChangeDetectionTask(this), CWelsSceneChangeDetectionTask);
    WELS_VERIFY_RETURN_IF (RET_OUTOFMEMORY, NULL == pTask)
    m_pcAllTaskList[CWelsProcessTask::WELS_PROCESS_TASK_SCENECHAGEDETECT]->push_back (pTask);
  }
  
  //fprintf(stdout, "CWelsProcessTaskManage CreateTasks m_iThreadNum %d kiTaskCount=%d\n", m_iThreadNum, kiTaskCount);
  return RET_SUCCESS;
}

void CWelsProcessTaskManage::DestroyTaskList (TASKLIST_TYPE* pTargetTaskList) {
  //fprintf(stdout, "CWelsProcessTaskManage: pTargetTaskList size=%d m_iTotalTaskNum=%d\n", static_cast<int32_t> (pTargetTaskList->size()), m_iTotalTaskNum);
  while (NULL != pTargetTaskList->begin()) {
    CWelsProcessTask* pTask = pTargetTaskList->begin();
    WELS_DELETE_OP (pTask);
    pTargetTaskList->pop_front();
  }
  pTargetTaskList = NULL;
}

void CWelsProcessTaskManage::DestroyTasks() {
  for (int32_t iIdx = 0; iIdx < CWelsProcessTask::WELS_PROCESS_TASK_ALL; iIdx++)  {
    DestroyTaskList (m_pcAllTaskList[iIdx]);
  }
  //fprintf(stdout, "[MT] CWelsProcessTaskManage() DestroyTasks, cleaned %d tasks\n", m_iTotalTaskNum);
}

void  CWelsProcessTaskManage::OnTaskMinusOne() {
  WelsCommon::CWelsAutoLock cAutoLock (m_cWaitTaskNumLock);
  m_iWaitTaskNum --;
  if (m_iWaitTaskNum <= 0) {
    WelsEventSignal (&m_hTaskEvent);
    //fprintf(stdout, "OnTaskMinusOne WelsEventSignal m_iWaitTaskNum=%d\n", m_iWaitTaskNum);
  }
  //fprintf(stdout, "OnTaskMinusOne m_iWaitTaskNum=%d\n", m_iWaitTaskNum);
}

int  CWelsProcessTaskManage::OnTaskCancelled() {
  OnTaskMinusOne();
  return 0;
}

int  CWelsProcessTaskManage::OnTaskExecuted() {
  OnTaskMinusOne();
  return 0;
}

EResult  CWelsProcessTaskManage::ExecuteTaskList (TASKLIST_TYPE* pTaskList) {
  m_iWaitTaskNum = m_iTaskNum;
  //fprintf(stdout, "ExecuteTaskList m_iWaitTaskNum=%d\n", m_iWaitTaskNum);
  if (0 == m_iWaitTaskNum) {
    return RET_SUCCESS;
  }
  
  int32_t iCurrentTaskCount = m_iWaitTaskNum; //if directly use m_iWaitTaskNum in the loop make cause sync problem
  int32_t iIdx = 0;
  while (iIdx < iCurrentTaskCount) {
    m_pThreadPool->QueueTask (pTaskList->GetIndexNode (iIdx));
    iIdx ++;
  }
  WelsEventWait (&m_hTaskEvent);
  
  return RET_SUCCESS;
}


EResult  CWelsProcessTaskManage::ExecuteTasks (const CWelsProcessTask::ETaskType iTaskType) {
  int32_t iPartitionNum = m_pThreadPool->GetThreadNum();
  
  
  return ExecuteTaskList (m_pcAllTaskList[iTaskType]);
}

int32_t  CWelsProcessTaskManage::GetThreadPoolThreadNum() {
  return m_pThreadPool->GetThreadNum();
}


WELSVP_NAMESPACE_END




