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

#include "param_svc.h"
#include "utils.h"

namespace WelsEnc {

class IWelsParametersetStrategy {
 public:
  virtual ~IWelsParametersetStrategy() { }

  static IWelsParametersetStrategy* CreateParametersetStrategy (EParameterSetStrategy eSpsPpsIdStrategy,
      const bool bSimulcastAVC, const int32_t kiSpatialLayerNum);

  //virtual SParaSetOffset* GetParaSetOffset() = 0;

  virtual int32_t GetPpsIdOffset (const int32_t iPpsId) = 0;
  virtual int32_t GetSpsIdOffset (const int32_t iPpsId, const int32_t iSpsId) = 0;
  virtual int32_t* GetSpsIdOffsetList (const int iParasetType) = 0;
  
  virtual uint32_t GetAllNeededParasetNum() = 0;

  virtual uint32_t GetNeededSpsNum() = 0;
  virtual uint32_t GetNeededSubsetSpsNum() = 0;
  virtual uint32_t GetNeededPpsNum() = 0;

  virtual void Update (const uint32_t kuiId, const int iParasetType) = 0;
  virtual void UpdatePpsList (sWelsEncCtx* pCtx) = 0;

  virtual bool CheckParamCompatibility (SWelsSvcCodingParam* pCodingParam, SLogContext* pLogCtx) = 0;

  virtual bool GenerateNewSps (sWelsEncCtx* pCtx, const bool kbUseSubsetSps, const int32_t iDlayerIndex,
                               const int32_t iDlayerCount, const int32_t kiSpsId,
                               SWelsSPS*& pSps, SSubsetSps*& pSubsetSps, bool bSVCBaselayer) = 0;

  virtual void InitPps (sWelsEncCtx* pCtx, uint32_t kiSpsId,
                        SWelsPPS* pPps,
                        SWelsSPS* pSps,
                        SSubsetSps* pSubsetSps,
                        const uint32_t kuiPpsId,
                        const bool kbDeblockingFilterPresentFlag,
                        const bool kbUsingSubsetSps,
                        const bool kbEntropyCodingModeFlag) = 0;

  virtual  void SetUseSubsetFlag (const uint32_t iPpsId, const bool bUseSubsetSps) = 0;

  virtual  void UpdateParaSetNum (sWelsEncCtx* pCtx) = 0;

  virtual  int32_t GetCurrentPpsId (const int32_t iPpsId, const int32_t iIdrLoop) = 0;

  virtual  void OutputCurrentStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList,
                                        sWelsEncCtx* pCtx, SExistingParasetList* pExistingParasetList) = 0;
  virtual  void LoadPreviousStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList) = 0;

  virtual int32_t GetSpsIdx (const int32_t iIdx) = 0;
 protected:
  virtual void UpdateParam (const bool bSimulcastAVC,
                            const int32_t kiSpatialLayerNum) = 0;
};


class  CWelsParametersetIdConstant : public IWelsParametersetStrategy {
 public:

  CWelsParametersetIdConstant();
  virtual ~ CWelsParametersetIdConstant();

  int32_t GetPpsIdOffset (const int32_t iPpsId);
  int32_t GetSpsIdOffset (const int32_t iPpsId, const int32_t iSpsId);
  int32_t* GetSpsIdOffsetList (const int iParasetType);
  
  uint32_t GetAllNeededParasetNum();

  uint32_t GetNeededSpsNum();
  uint32_t GetNeededSubsetSpsNum();
  uint32_t GetNeededPpsNum();

  void LoadPrevious (SExistingParasetList* pExistingParasetList, SWelsSPS* pSpsArray, SSubsetSps*       pSubsetArray,
                     SWelsPPS* pPpsArray);

  void Update (const uint32_t kuiId, const int iParasetType);
  void UpdatePpsList (sWelsEncCtx* pCtx) {};

  bool CheckParamCompatibility (SWelsSvcCodingParam* pCodingParam, SLogContext* pLogCtx) {
    return true;
  };

  bool GenerateNewSps (sWelsEncCtx* pCtx, const bool kbUseSubsetSps, const int32_t iDlayerIndex,
                       const int32_t iDlayerCount, const int32_t kiSpsId,
                       SWelsSPS*& pSps, SSubsetSps*& pSubsetSps, bool bSVCBaselayer);

  void InitPps (sWelsEncCtx* pCtx, uint32_t kiSpsId,
                SWelsPPS* pPps,
                SWelsSPS* pSps,
                SSubsetSps* pSubsetSps,
                const uint32_t kuiPpsId,
                const bool kbDeblockingFilterPresentFlag,
                const bool kbUsingSubsetSps,
                const bool kbEntropyCodingModeFlag);

  void SetUseSubsetFlag (const uint32_t iPpsId, const bool bUseSubsetSps);

  void UpdateParaSetNum (sWelsEncCtx* pCtx) {};

  int32_t GetCurrentPpsId (const int32_t iPpsId, const int32_t iIdrLoop) {
    return iPpsId;
  };

  void OutputCurrentStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList, sWelsEncCtx* pCtx,
                               SExistingParasetList* pExistingParasetList) {};
  void LoadPreviousStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList) {};

  int32_t GetSpsIdx (const int32_t iIdx) {
    return 0;
  };
 protected:
  void UpdateParam (const bool bSimulcastAVC,
                    const int32_t kiSpatialLayerNum);

  uint32_t GetBasicNeededSpsNum() {
    return 1;
  };
  uint32_t GetBasicNeededPpsNum();

  void LoadPreviousSps (SExistingParasetList* pExistingParasetList, SWelsSPS* pSpsArray,
                        SSubsetSps*       pSubsetArray) {};
  void LoadPreviousPps (SExistingParasetList* pExistingParasetList, SWelsPPS* pPpsArray) {};

  SParaSetOffset m_sParaSetOffset;
  bool m_bSimulcastAVC;
  int32_t m_iSpatialLayerNum;
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

class  CWelsParametersetIdNonConstant : public CWelsParametersetIdConstant {
 public:
  void OutputCurrentStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList, sWelsEncCtx* pCtx,
                               SExistingParasetList* pExistingParasetList);
  void LoadPreviousStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList);
};

class  CWelsParametersetIdIncreasing : public CWelsParametersetIdNonConstant {
 public:
  int32_t GetPpsIdOffset (const int32_t iPpsId);
  int32_t GetSpsIdOffset (const int32_t iPpsId, const int32_t iSpsId);

  void Update (const uint32_t kuiId, const int iParasetType);

 protected:
  uint32_t GetBasicNeededSpsNum() {
    return 1;
  };

  void ParasetIdAdditionIdAdjust (SParaSetOffsetVariable* sParaSetOffsetVariable, const int32_t kiCurEncoderParaSetId,
                                  const uint32_t kuiMaxIdInBs);

 private:
  void DebugPps (const int32_t kiPpsId);
  void DebugSpsPps (const int32_t iPpsId, const int32_t iSpsId);
};


class  CWelsParametersetSpsListing : public CWelsParametersetIdNonConstant {
 public:
  uint32_t GetNeededSubsetSpsNum();

  void LoadPrevious (SExistingParasetList* pExistingParasetList,  SWelsSPS* pSpsArray, SSubsetSps*       pSubsetArray,
                     SWelsPPS* pPpsArray);

  bool CheckParamCompatibility (SWelsSvcCodingParam* pCodingParam, SLogContext* pLogCtx);

  bool GenerateNewSps (sWelsEncCtx* pCtx, const bool kbUseSubsetSps, const int32_t iDlayerIndex,
                       const int32_t iDlayerCount, const int32_t kiSpsId,
                       SWelsSPS*& pSps, SSubsetSps*& pSubsetSps, bool bSVCBaselayer);

  void UpdateParaSetNum (sWelsEncCtx* pCtx);

  int32_t GetSpsIdx (const int32_t iIdx) {
    return iIdx;
  };

  void OutputCurrentStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList, sWelsEncCtx* pCtx,
                               SExistingParasetList* pExistingParasetList);
 protected:
  uint32_t GetBasicNeededSpsNum();
  void LoadPreviousSps (SExistingParasetList* pExistingParasetList, SWelsSPS* pSpsArray, SSubsetSps*       pSubsetArray);
  bool CheckPpsGenerating();
  int32_t SpsReset (sWelsEncCtx* pCtx, bool kbUseSubsetSps);
};

class  CWelsParametersetSpsPpsListing : public CWelsParametersetSpsListing {
 public:
  void UpdatePpsList (sWelsEncCtx* pCtx);

  void InitPps (sWelsEncCtx* pCtx, uint32_t kiSpsId,
                SWelsPPS* pPps,
                SWelsSPS* pSps,
                SSubsetSps* pSubsetSps,
                const uint32_t kuiPpsId,
                const bool kbDeblockingFilterPresentFlag,
                const bool kbUsingSubsetSps,
                const bool kbEntropyCodingModeFlag);

  void UpdateParaSetNum (sWelsEncCtx* pCtx);

  int32_t GetCurrentPpsId (const int32_t iPpsId, const int32_t iIdrLoop);

  void OutputCurrentStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList, sWelsEncCtx* pCtx,
                               SExistingParasetList* pExistingParasetList);
  void LoadPreviousStructure (SParaSetOffsetVariable* pParaSetOffsetVariable, int32_t* pPpsIdList);
 protected:

  uint32_t GetBasicNeededPpsNum();

  void LoadPreviousPps (SExistingParasetList* pExistingParasetList, SWelsPPS* pPpsArray);

  bool CheckPpsGenerating();
  int32_t SpsReset (sWelsEncCtx* pCtx, bool kbUseSubsetSps);
};

}

#endif
