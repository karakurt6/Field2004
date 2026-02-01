#ifndef __REDIRECT_H__
#define __REDIRECT_H__

class CRedirectOutput
{
public:
  virtual void Output(DWORD cBytes, LPCTSTR pData) = 0;
  BOOL Redirect(DWORD nStdHandle, FILE* pStream);
  void StopRedirect();

private:
  BOOL m_bDone;
  HANDLE m_hThread;
  HANDLE m_hReadPipe;
  HANDLE m_hWritePipe;

  static DWORD WINAPI ThreadProc(LPVOID pParam);
};

class CDebugRedirect: public CRedirectOutput
{
public:
  void Output(DWORD cBytes, LPCTSTR pData);
};

class CEditRedirect: public CEdit, public CRedirectOutput
{
public:
  void Output(DWORD cBytes, LPCTSTR pData);
};

#endif
