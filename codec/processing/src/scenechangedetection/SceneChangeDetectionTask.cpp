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
 * \file    wels_task_encoder.h
 *
 * \brief   interface for encoder tasks
 *
 * \date    07/06/2015 Created
 *
 *************************************************************************************
 */

#include <string.h>
#include <assert.h>

#include "typedefs.h"
#include "utils.h"
#include "measure_time.h"
#include "WelsTask.h"

#include "wels_process_task_base.h"
#include "wels_task_preprocess.h"
#include "WelsThreadLib.h"

WELSVP_NAMESPACE_BEGIN

CWelsSceneChangeDetectionTask::CWelsSceneChangeDetectionTask (WelsCommon::IWelsTaskSink* pSink) : CWelsProcessTask(pSink), m_eTaskResult (RET_SUCCESS) {

}

CWelsSceneChangeDetectionTask::~CWelsSceneChangeDetectionTask() {
}

int32_t CWelsSceneChangeDetectionTask::Execute() {
  WelsThreadSetName ("OpenH264Enc_CWelsSceneChangeDetectionTask_Execute");
  return m_eTaskResult;
}



WELSVP_NAMESPACE_END


