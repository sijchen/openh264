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
 * \file    wels_process_task_base.h
 *
 * \brief   interface for process base task
 *
 * \date    12/10/2015 Created
 *
 *************************************************************************************
 */



#ifndef  _WELS_PROCESS_BASE_TASK_H_
#define  _WELS_PROCESS_BASE_TASK_H_

#include "typedef.h"
#include "WelsTask.h"
#include "WelsFrameWork.h"

#include "ScrollDetectionFuncs.h"


WELSVP_NAMESPACE_BEGIN

class CWelsProcessTask : public WelsCommon::IWelsTask {
 public:
  enum ETaskType {
    WELS_PROCESS_TASK_NONE = 0,
    WELS_PROCESS_TASK_SCENECHAGEDETECT = 1,
    WELS_PROCESS_TASK_ALL = 2,
  };

  CWelsProcessTask (WelsCommon::IWelsTaskSink* pSink): IWelsTask (pSink), m_bSeparatedFlag(false) {};
  virtual ~CWelsProcessTask() {};

  virtual uint32_t GetTaskNum(uint32_t iNumber = 0) {
    if (iNumber!=0) {
      m_iTaskNum=iNumber;
    }
    return iNumber;
  };

  void GetProperPixMap(IStrategy* pStrategy, int32_t iType, int32_t iIdx, SPixMap* pSrcPixMap,
                       SPixMap* pDstPixMap);
  void UpdatePixMap (IStrategy* pStrategy, int32_t iType, SPixMap& pSrcPixMap, SPixMap& pRefPixMap);

  int32_t Execute();

 protected:
  IStrategy* m_pStrategy;
  SPixMap m_pSrcPixMap;
  SPixMap m_pRefPixMap;
  int32_t m_iType;
  EMethods m_eMethod;
  bool m_bSeparatedFlag;
  
  int32_t m_iTaskNum;
};

WELSVP_NAMESPACE_END
#endif




