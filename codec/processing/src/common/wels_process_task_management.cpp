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
#include "memory_align.h"

#include "wels_process_task_base.h"
#include "wels_process_task_management.h"

#include "../denoise/denoise.h"
#include "../downsample/downsample.h"
#include "../scrolldetection/ScrollDetection.h"
#include "../vaacalc/vaacalculation.h"
#include "../backgrounddetection/BackgroundDetection.h"
#include "../adaptivequantization/AdaptiveQuantization.h"
#include "../complexityanalysis/ComplexityAnalysis.h"
#include "../imagerotate/imagerotate.h"

WELSVP_NAMESPACE_BEGIN

IWelsProcessTaskManage*   IWelsProcessTaskManage::CreateProcessTaskManage() {

  IWelsProcessTaskManage* pTaskManage;
  pTaskManage = WELS_NEW_OP (CWelsProcessTaskManage(), CWelsProcessTaskManage);
  WELS_VERIFY_RETURN_IF (NULL, NULL == pTaskManage)

  if (RET_SUCCESS != pTaskManage->Init()) {
    pTaskManage->Uninit();
    WELS_DELETE_OP (pTaskManage);
  }
  return pTaskManage;
}

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

EResult CWelsProcessTaskManage::Init() {
  //fprintf(stdout, "m_pThreadPool = &(CWelsThreadPool::GetInstance, this=%x\n", this);

  m_pThreadPool = & (WelsCommon::CWelsThreadPool::AddReference());
  WELS_VERIFY_RETURN_IF (RET_OUTOFMEMORY, NULL == m_pThreadPool)
  //fprintf(stdout, "m_pThreadPool = &(CWelsThreadPool::GetInstance3\n");

  return CreateTasks (m_pThreadPool->GetThreadNum());
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
  int32_t iPartitionNum = m_pThreadPool->GetThreadNum();
  //printf("iPartitionNum=%d\n", iPartitionNum );

  m_pcAllTaskList[0] = new TASKLIST_TYPE();
  WELS_VERIFY_RETURN_IF (RET_OUTOFMEMORY, NULL == m_pcAllTaskList[0])

  for (int idx = 0; idx < iPartitionNum; idx++) {
    //pTask = (CWelsProcessTask*) (new CWelsProcessTask (this));
    pTask = new CWelsProcessTask (this);
    //pTask = WELS_NEW_OP (CWelsProcessTask(this), CWelsProcessTask);
    WELS_VERIFY_RETURN_IF (RET_OUTOFMEMORY, NULL == pTask)
    m_pcAllTaskList[0]->push_back (pTask);
  }

  //fprintf(stdout, "CWelsProcessTaskManage CreateTasks m_iThreadNum %d kiTaskCount=%d\n", m_iThreadNum, kiTaskCount);
  return RET_SUCCESS;
}

void CWelsProcessTaskManage::DestroyTasks() {
  //for (int32_t iIdx = 0; iIdx < CWelsProcessTask::WELS_PROCESS_TASK_ALL; iIdx++)  {
  int32_t iIdx = 0;
  while (NULL != m_pcAllTaskList[iIdx]->begin()) {
    CWelsProcessTask* pTask = m_pcAllTaskList[iIdx]->begin();
    WELS_DELETE_OP (pTask);
    m_pcAllTaskList[iIdx]->pop_front();
  }
  WELS_DELETE_OP (m_pcAllTaskList[iIdx]);
  //}
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


void GetPartitionOfPixMap (int32_t iIdx, int32_t iTotal, SPixMap& sWholePixMap, SPixMap* pPartPixMap) {
  memcpy (pPartPixMap, &sWholePixMap, sizeof (SPixMap));

  int32_t iPartitionHeight = ((sWholePixMap.sRect.iRectHeight >> 3) / iTotal) << 3;  //many processing are with x8 blocks

  if (sWholePixMap.iSizeInBits != 8) {
    return;
  }
  //To Volvet: arithmetic on a void* is illegal in both C and C++
  pPartPixMap->pPixel[0] = (uint8_t*) (sWholePixMap.pPixel[0]) + (iIdx * iPartitionHeight) * sWholePixMap.iStride[0];
  pPartPixMap->pPixel[1] = (uint8_t*)sWholePixMap.pPixel[1] + (iIdx * iPartitionHeight >> 1) *
                           sWholePixMap.iStride[1];
  pPartPixMap->pPixel[2] = (uint8_t*)sWholePixMap.pPixel[2] + (iIdx * iPartitionHeight >> 1) *
                           sWholePixMap.iStride[2];

  //iRectHeight
  pPartPixMap->sRect.iRectHeight = iPartitionHeight;
  if (iIdx == iTotal - 1) {
    pPartPixMap->sRect.iRectHeight = sWholePixMap.sRect.iRectHeight - (iIdx - 1) * iPartitionHeight;
  }
}

void GetTransposePartitionOfPixMap (int32_t iIdx, int32_t iTotal, SPixMap& sWholePixMap, SPixMap* pPartPixMap) {
  memcpy (pPartPixMap, &sWholePixMap, sizeof (SPixMap));

  int32_t iPartitionWidth = ((sWholePixMap.sRect.iRectWidth >> 3) / iTotal) <<
                            3;  //many processsing in vp is based on x8 blocks

  //To Volvet: arithmetic on a void* is illegal in both C and C++
  if (sWholePixMap.iSizeInBits != 8) {
    return;
  }
  pPartPixMap->pPixel[0] = (uint8_t*) (sWholePixMap.pPixel[0]) + (iIdx * iPartitionWidth);
  pPartPixMap->pPixel[1] = (uint8_t*) (sWholePixMap.pPixel[1]) + (iIdx * iPartitionWidth >> 1);
  pPartPixMap->pPixel[2] = (uint8_t*) (sWholePixMap.pPixel[2]) + (iIdx * iPartitionWidth >> 1);

  if (iIdx != iTotal - 1) {
    pPartPixMap->sRect.iRectHeight = iPartitionWidth;
  } else {
    pPartPixMap->sRect.iRectHeight = sWholePixMap.sRect.iRectHeight - iIdx * iPartitionWidth;
  }
}

EResult  CWelsProcessTaskManage::ExecuteTasks (IStrategy* pStrategy, int32_t iType, SPixMap* pSrcPixMap,
    SPixMap* pDstPixMap) {
  /*METHOD_DENOISE              ,
  METHOD_SCENE_CHANGE_DETECTION_VIDEO ,
  METHOD_SCENE_CHANGE_DETECTION_SCREEN ,
  METHOD_DOWNSAMPLE            ,
  METHOD_VAA_STATISTICS        ,
  METHOD_BACKGROUND_DETECTION  ,
  METHOD_ADAPTIVE_QUANT ,
  METHOD_COMPLEXITY_ANALYSIS   ,
  METHOD_COMPLEXITY_ANALYSIS_SCREEN,
  METHOD_IMAGE_ROTATE          ,
  METHOD_SCROLL_DETECTION,
  METHOD_MASK*/
  if (METHOD_IMAGE_ROTATE == pStrategy->m_eMethod || pSrcPixMap->eFormat != VIDEO_FORMAT_420
      || pDstPixMap->eFormat != VIDEO_FORMAT_420) {
    return RET_NOTSUPPORTED;
  }

  m_iWaitTaskNum = m_pcAllTaskList[0]->size();
  //fprintf(stdout, "ExecuteTaskList m_iWaitTaskNum=%d\n", m_iWaitTaskNum);
  if (0 == m_iWaitTaskNum) {
    return RET_SUCCESS;
  }
  for (int32_t iIdx = 0; iIdx < m_iWaitTaskNum; iIdx++) {
    SPixMap sTarDstPixMap;
    SPixMap sTarSrcPixMap;
    if (METHOD_IMAGE_ROTATE == pStrategy->m_eMethod) {
      GetTransposePartitionOfPixMap (iIdx, m_iWaitTaskNum, *pDstPixMap, &sTarDstPixMap);
    } else {
      GetPartitionOfPixMap (iIdx, m_iWaitTaskNum, *pDstPixMap, &sTarDstPixMap);
    }
    GetPartitionOfPixMap (iIdx, m_iWaitTaskNum, *pSrcPixMap, &sTarSrcPixMap);

    m_pcAllTaskList[0]->GetIndexNode (iIdx)->UpdatePixMap (pStrategy, iType, sTarSrcPixMap, sTarDstPixMap);

  }

  int32_t iCurrentTaskCount = m_iWaitTaskNum; //if directly use m_iWaitTaskNum in the loop make cause sync problem
  int32_t iIdx = 0;
  while (iIdx < iCurrentTaskCount) {
    m_pThreadPool->QueueTask (m_pcAllTaskList[0]->GetIndexNode (iIdx));
    iIdx ++;
  }
  WelsEventWait (&m_hTaskEvent);

  return RET_SUCCESS;
}

int32_t  CWelsProcessTaskManage::GetThreadPoolThreadNum() {
  return m_pThreadPool->GetThreadNum();
}


WELSVP_NAMESPACE_END




