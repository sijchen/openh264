/**
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsThread.cpp
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *
 *
 *
 * \brief	define for thread
 *
 * \date	5/09/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/

#include "WelsThread.h"

namespace WelsCommon {

CWelsThread::CWelsThread() :
  m_hThread (0),
  m_bRunning (false),
  m_bEndFlag (false) {
  WELS_THREAD_ERROR_CODE rc = WelsEventOpen (&m_hEvent, "WelsSem");

  if (WELS_THREAD_ERROR_OK != rc) {
    m_hEvent = NULL;
  }
}

CWelsThread::~CWelsThread() {
  Kill();
  WelsEventClose (&m_hEvent, "WelsSem");
  m_hEvent = NULL;
}

void CWelsThread::Thread() {
  while (true) {
    WelsEventWait (&m_hEvent);

    if (GetEndFlag()) {
      break;
    }

    ExecuteTask();
  }

  SetRunning (false);
}

WELS_THREAD_ERROR_CODE CWelsThread::Start() {
  if (NULL == m_hEvent) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  if (GetRunning()) {
    return WELS_THREAD_ERROR_OK;
  }

  SetEndFlag (false);

  WELS_THREAD_ERROR_CODE rc = WelsThreadCreate (&m_hThread,
                              (LPWELS_THREAD_ROUTINE)TheThread, this, 0);

  if (WELS_THREAD_ERROR_OK != rc) {
    return rc;
  }

  while (!GetRunning()) {
    WelsSleep (1);
  }

  return WELS_THREAD_ERROR_OK;
}

void CWelsThread::Kill() {
  if (!GetRunning()) {
    return;
  }

  SetEndFlag (true);

  WelsEventSignal (&m_hEvent);
  WelsThreadJoin (m_hThread);
  return;
}

WELS_THREAD_ROUTINE_TYPE  CWelsThread::TheThread (void* pParam) {
  CWelsThread* pThis = static_cast<CWelsThread*> (pParam);

  pThis->SetRunning (true);

  pThis->Thread();

  WELS_THREAD_ROUTINE_RETURN (NULL);
}

}


