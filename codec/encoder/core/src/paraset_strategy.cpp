/*!
 * \copy
 *     Copyright (c)  2013, Cisco Systems
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
 */

#
#include "encoder_context.h"
#include "paraset_strategy.h"

namespace WelsEnc {


IWelsParametersetIdStrategy*   IWelsParametersetIdStrategy::CreateParametersetStrategy (
  EParameterSetStrategy eSpsPpsIdStrategy) {

  IWelsParametersetIdStrategy* pParametersetIdStrategy = NULL;
  switch (eSpsPpsIdStrategy) {
  case INCREASING_ID:
    break;
  case SPS_LISTING:
    break;
  case SPS_LISTING_AND_PPS_INCREASING:
    break;
  case SPS_PPS_LISTING:
    break;
  case CONSTANT_ID:
  default:
    pParametersetIdStrategy = WELS_NEW_OP (CWelsParametersetIdConstant(), CWelsParametersetIdConstant);
    WELS_VERIFY_RETURN_IF (NULL, NULL == pParametersetIdStrategy)
    break;
  }
  return pParametersetIdStrategy;
}


CWelsParametersetIdConstant::CWelsParametersetIdConstant() {
}

CWelsParametersetIdConstant::~CWelsParametersetIdConstant() {
}


CWelsParametersetIdIncreasing::CWelsParametersetIdIncreasing() {
}

CWelsParametersetIdIncreasing::~CWelsParametersetIdIncreasing() {
}

void CWelsParametersetIdIncreasing::DebugPps (SWelsPPS* pPps) {
  //#if _DEBUG
  //SParaSetOffset use, 110421
  //if ( (INCREASING_ID & pCtx->pPSOVector->eSpsPpsIdStrategy)) {
  const int32_t kiParameterSetType = (m_sParaSetOffset.bPpsIdMappingIntoSubsetsps[pPps->iPpsId] ?
                                      PARA_SET_TYPE_SUBSETSPS : PARA_SET_TYPE_AVCSPS) ;

  const int32_t kiTmpSpsIdInBs = pPps->iSpsId +
                                 m_sParaSetOffset.sParaSetOffsetVariable[kiParameterSetType].iParaSetIdDelta[pPps->iSpsId];
  const int32_t tmp_pps_id_in_bs = pPps->iPpsId +
                                   m_sParaSetOffset.sParaSetOffsetVariable[PARA_SET_TYPE_PPS].iParaSetIdDelta[pPps->iPpsId];
  assert (MAX_SPS_COUNT > kiTmpSpsIdInBs);
  assert (MAX_PPS_COUNT > tmp_pps_id_in_bs);
  assert (m_sParaSetOffset.sParaSetOffsetVariable[kiParameterSetType].bUsedParaSetIdInBs[kiTmpSpsIdInBs]);
  //}
  //#endif
}

}
