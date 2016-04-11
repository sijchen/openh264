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

void GetPartitionOfPixMap (int32_t iIdx, int32_t iTotal, SPixMap& sWholePixMap, SPixMap* pPartPixMap) {
  memcpy (pPartPixMap, &sWholePixMap, sizeof (SPixMap));
  
  int32_t iPartitionHeight = ((sWholePixMap.sRect.iRectHeight >> 3) / iTotal) << 3;  //many processing are with x8 blocks
  if (sWholePixMap.iSizeInBits != 8) {
    return;
  }
  pPartPixMap->pPixel[0] = (uint8_t*) (sWholePixMap.pPixel[0]) + (iIdx * iPartitionHeight) * sWholePixMap.iStride[0];
  pPartPixMap->pPixel[1] = (uint8_t*)sWholePixMap.pPixel[1] + (iIdx * iPartitionHeight >> 1) *
  sWholePixMap.iStride[1];
  pPartPixMap->pPixel[2] = (uint8_t*)sWholePixMap.pPixel[2] + (iIdx * iPartitionHeight >> 1) *
  sWholePixMap.iStride[2];
  
  //iRectHeight
  pPartPixMap->sRect.iRectHeight = iPartitionHeight;
  if (iIdx == iTotal - 1) {
    pPartPixMap->sRect.iRectHeight = sWholePixMap.sRect.iRectHeight - iIdx * iPartitionHeight;
  }
}

void GetTransposePartitionOfPixMap (int32_t iIdx, int32_t iTotal, SPixMap& sWholePixMap, SPixMap* pPartPixMap) {
  memcpy (pPartPixMap, &sWholePixMap, sizeof (SPixMap));
  
  int32_t iPartitionWidth = ((sWholePixMap.sRect.iRectWidth >> 3) / iTotal) <<
  3;  //many processsing in vp is based on x8 blocks
  
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

void CWelsProcessTask::GetProperPixMap(IStrategy* pStrategy, int32_t iType, int32_t iIdx, SPixMap* pSrcPixMap,
                                       SPixMap* pDstPixMap) {
  int32_t iTaskNum = GetTaskNum();
  SPixMap sTarDstPixMap;
  SPixMap sTarSrcPixMap;
  //if (METHOD_IMAGE_ROTATE == pStrategy->m_eMethod && pDstPixMap) {
  //  GetTransposePartitionOfPixMap (iIdx, m_iWaitTaskNum, *pDstPixMap, &sTarDstPixMap);
  //} else
  if (pDstPixMap) {
    GetPartitionOfPixMap (iIdx, iTaskNum, *pDstPixMap, &sTarDstPixMap);
  }
  GetPartitionOfPixMap (iIdx, iTaskNum, *pSrcPixMap, &sTarSrcPixMap);
  
  UpdatePixMap (pStrategy, iType, sTarSrcPixMap, sTarDstPixMap);
  
}


void CWelsProcessTask::UpdatePixMap (IStrategy* pStrategy, int32_t iType, SPixMap& pSrcPixMap, SPixMap& pRefPixMap) {
  m_pStrategy = pStrategy;
  m_pSrcPixMap = pSrcPixMap;
  m_pRefPixMap = pRefPixMap;
  m_iType = iType;
  
  m_eMethod    = WelsVpGetValidMethod (iType);
};



int32_t CWelsProcessTask::Execute() {
  WelsThreadSetName ("OpenH264Enc_CWelsProcessTask_Process");
  return m_pStrategy->ProcessPart (m_iType, &m_pSrcPixMap, &m_pRefPixMap, NULL);
}



WELSVP_NAMESPACE_END




