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

#ifndef WELS_PARASET_STRATEGY_H
#define WELS_PARASET_STRATEGY_H

namespace WelsEnc {

class IWelsParametersetIdStrategy {
 public:
  virtual ~IWelsParametersetIdStrategy() { }

  static IWelsParametersetIdStrategy* CreateParametersetStrategy (EParameterSetStrategy eSpsPpsIdStrategy);

  virtual void DebugPps (SWelsPPS* pPps) = 0;

  virtual SParaSetOffset* GetParaSetOffset() = 0;
};


class  CWelsParametersetIdConstant : public IWelsParametersetIdStrategy {
 public:

  CWelsParametersetIdConstant();
  virtual ~ CWelsParametersetIdConstant();

  void DebugPps (SWelsPPS* pPps) {};

};

/*
  typedef struct TagParaSetOffsetVariable {
    int32_t       iParaSetIdDelta[MAX_DQ_LAYER_NUM+1];//mark delta between SPS_ID_in_bs and sps_id_in_encoder, can be minus, for each dq-layer
    //need not extra +1 due no MGS and FMO case so far
    bool          bUsedParaSetIdInBs[MAX_PPS_COUNT];      //mark the used SPS_ID with 1
    uint32_t      uiNextParaSetIdToUseInBs;               //mark the next SPS_ID_in_bs, for all layers
  } SParaSetOffsetVariable;

  typedef struct TagParaSetOffset {
    //in PS0 design, "sParaSetOffsetVariable" record the previous paras before current IDR, AND NEED to be stacked and recover across IDR
    SParaSetOffsetVariable
    sParaSetOffsetVariable[PARA_SET_TYPE]; //PARA_SET_TYPE=3; paraset_type = 0: AVC_SPS; =1: Subset_SPS; =2: PPS
    //in PSO design, "bPpsIdMappingIntoSubsetsps" uses the current para of current IDR period
    bool
    bPpsIdMappingIntoSubsetsps[MAX_DQ_LAYER_NUM+1];   // need not extra +1 due no MGS and FMO case so far

    int32_t  iPpsIdList[MAX_DQ_LAYER_NUM][MAX_PPS_COUNT]; //index0: max pps types; index1: for differnt IDRs, if only index0=1, index1 can reach MAX_PPS_COUNT

    //#if _DEBUG
    int32_t  eSpsPpsIdStrategy;
    //#endif

    uint32_t uiNeededSpsNum;
    uint32_t uiNeededSubsetSpsNum;
    uint32_t uiNeededPpsNum;

    uint32_t uiInUseSpsNum;
    uint32_t uiInUseSubsetSpsNum;
    uint32_t uiInUsePpsNum;
  } SParaSetOffset;
  */


class  CWelsParametersetIdIncreasing : public IWelsParametersetIdStrategy {
 public:

  CWelsParametersetIdIncreasing();
  virtual ~ CWelsParametersetIdIncreasing();

  void DebugPps (SWelsPPS* pPps);

 private:
  SParaSetOffset m_sParaSetOffset;
};

}

#endif
