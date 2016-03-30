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
 * \file    wels_process_task_base.cpp
 *
 * \brief   function for task process management
 *
 * \date    12/10/2015 Created
 *
 *************************************************************************************
 */

#include "wels_process_task_base.h"
WELSVP_NAMESPACE_BEGIN

void CWelsProcessTask::UpdatePixMap (IStrategy* pStrategy, int32_t iType, SPixMap& pSrcPixMap, SPixMap& pRefPixMap) {
  m_pStrategy = pStrategy;
  m_pSrcPixMap = pSrcPixMap;
  m_pRefPixMap = pRefPixMap;
  m_iType = iType;
};
int32_t CWelsProcessTask::Execute() {
  WelsThreadSetName ("OpenH264Enc_CWelsProcessTask_Execute");
  return m_pStrategy->Process (m_iType, &m_pSrcPixMap, &m_pRefPixMap);
}
WELSVP_NAMESPACE_END




