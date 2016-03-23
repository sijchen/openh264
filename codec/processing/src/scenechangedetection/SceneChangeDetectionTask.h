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
 * \file    wels_task_process.h
 *
 * \brief   interface for encoder tasks
 *
 * \date    12/10/2015 Created
 *
 *************************************************************************************
 */

#ifndef _WELS_PROCESS_TASK_H_
#define _WELS_PROCESS_TASK_H_

#include "IWelsVP.h"
#include "wels_process_task_base.h"
#include "SceneChangeDetection.h"
WELSVP_NAMESPACE_BEGIN


class CWelsSceneChangeDetectionTask : public CWelsProcessTask {
 public:
  CWelsSceneChangeDetectionTask (WelsCommon::IWelsTaskSink* pSink);
  virtual ~CWelsSceneChangeDetectionTask();

  virtual int32_t Execute();

  virtual uint32_t        GetTaskType() const {
    return WELS_PROCESS_TASK_SCENECHAGEDETECT;
  }
 protected:
  EResult m_eTaskResult;

  int32_t m_iStartMbIdx;
  int32_t m_iEndMbIdx;

  uint32_t m_uiDependencyId;
};


WELSVP_NAMESPACE_END
#endif  //header guard

