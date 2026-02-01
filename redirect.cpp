#include "stdafx.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "redirect.h"

BOOL CRedirectOutput::Redirect(DWORD nStdHandle, FILE* pStream)
{
  DWORD nThreadID;
  m_hThread = CreateThread(NULL, 0, ThreadProc, this, CREATE_SUSPENDED, &nThreadID);
  if (m_hThread == NULL)
    return FALSE;

  if (!SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL))
  {
    CloseHandle(m_hThread);
    return FALSE;
  }

  if (!CreatePipe(&m_hReadPipe, &m_hWritePipe, NULL, 0))
  {
    CloseHandle(m_hThread);
    return FALSE;
  }

  if (!SetStdHandle(nStdHandle, m_hWritePipe))
  {
    CloseHandle(m_hThread);
    CloseHandle(m_hReadPipe);
    CloseHandle(m_hWritePipe);
    return FALSE;
  }

  int hConHandle = _open_osfhandle((intptr_t) m_hWritePipe, _O_TEXT);
  FILE *fp = _fdopen(hConHandle, "w");
  *pStream = *fp;
  setvbuf(pStream, NULL, _IONBF, 0);

  if (ResumeThread(m_hThread) == -1)
  {
    CloseHandle(m_hThread);
    CloseHandle(m_hReadPipe);
    CloseHandle(m_hWritePipe);
    return FALSE;
  }

  m_bDone = FALSE;
  return TRUE;
}

DWORD WINAPI CRedirectOutput::ThreadProc(LPVOID pParam)
{
  CRedirectOutput* pRedirect = (CRedirectOutput*) pParam;

  while (!pRedirect->m_bDone)
  {
    while (true)
    {
      TCHAR chBuff[16];
      DWORD dwRead = 0;
      if (!ReadFile(pRedirect->m_hReadPipe, chBuff, sizeof(chBuff)-1, &dwRead, NULL))
        break;
      if (dwRead == 0)
        break;

      chBuff[dwRead] = _T('\0');
      pRedirect->Output(dwRead, chBuff);
    }
  }
  return 0;
}

void CRedirectOutput::StopRedirect()
{
  m_bDone = TRUE;
  CloseHandle(m_hWritePipe);
  CloseHandle(m_hReadPipe);
  SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
  WaitForSingleObject(m_hThread, INFINITE);
  CloseHandle(m_hThread);
}

void CDebugRedirect::Output(DWORD cBytes, LPCTSTR pData)
{
  OutputDebugString(pData);
}

void CEditRedirect::Output(DWORD cBytes, LPCTSTR pData)
{
  CString sText;
  GetWindowText(sText);
  sText += pData;
  SetWindowText(sText);
}
