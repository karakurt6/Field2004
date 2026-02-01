// Field.h : main header file for the FIELD application
//

#if !defined(AFX_FIELD_H__4FE7FB13_82A3_4926_A61B_C77CE5AB1F1B__INCLUDED_)
#define AFX_FIELD_H__4FE7FB13_82A3_4926_A61B_C77CE5AB1F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <algorithm>
#include <lmclient.h>
using std::min;
using std::max;

class COverView;
class CFieldForm;
class CFieldView;
class CCrossSection;
class CProbeDialog;
class CCumulativeS;
class CResDialog;
class CIntegralProp;
class CFindWell;
class CLogtrackType;

/////////////////////////////////////////////////////////////////////////////
// CFieldApp:
// See Field.cpp for the implementation of this class
//

class CDebugRedirect;

class CFieldApp : public CWinApp
{
  CDebugRedirect *m_pDebugRedirect;

public:
	CFieldApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFieldApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
	LM_HANDLE* m_jobLicense;
	bool m_bDemoMode;

// Implementation
	//{{AFX_MSG(CFieldApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
};

COverView* GetOverView();
CFieldForm* GetFieldForm();
CFieldView* GetFieldView();
CCrossSection* GetCrossSection();
CProbeDialog* GetProbeDialog();
CCumulativeS* GetCumulativeS();
CResDialog* GetResDialog();
CIntegralProp* GetIntegralProp();
CFindWell* GetFindWell();
CLogtrackType* GetLogtrackType();

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIELD_H__4FE7FB13_82A3_4926_A61B_C77CE5AB1F1B__INCLUDED_)
