// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__3D9652D0_915B_46CB_98FE_8E6DAAD16817__INCLUDED_)
#define AFX_MAINFRM_H__3D9652D0_915B_46CB_98FE_8E6DAAD16817__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CrossSection.h"
#include "ProbeDialog.h"
#include "CumulativeS.h"
#include "ResDialog.h"
#include "IntegralProp.h"
#include "FindWell.h"
#include "LogtrackType.h"

class CFieldForm;
class CFieldView;
class COverView;

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
  CSplitterWnd m_wndSplitter1;
  CSplitterWnd m_wndSplitter2;

  CFieldForm *m_pFieldForm;
  COverView *m_pOverView;
  CFieldView *m_pFieldView;
  CCrossSection m_wndCrossSection;
  CProbeDialog m_wndProbeDialog;
  CCumulativeS m_wndCumulativeS;
  CResDialog m_wndResDialog;
  CIntegralProp m_wndIntegralProp;
  CFindWell m_wndFindWell;
	CLogtrackType m_wndLogtrackType;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewCrossSection();
	afx_msg void OnUpdateViewCrossSection(CCmdUI* pCmdUI);
	afx_msg void OnViewProbeGrid();
	afx_msg void OnUpdateViewProbeGrid(CCmdUI* pCmdUI);
	afx_msg void OnSaturation();
	afx_msg void OnUpdateSaturation(CCmdUI* pCmdUI);
	afx_msg void OnResdialog();
	afx_msg void OnUpdateResdialog(CCmdUI* pCmdUI);
	afx_msg void OnFieldExpr();
	afx_msg void OnUpdateFieldExpr(CCmdUI* pCmdUI);
	afx_msg void OnFindwell();
	afx_msg void OnUpdateFindwell(CCmdUI* pCmdUI);
	afx_msg void OnViewWellmatch();
	afx_msg void OnUpdateViewWellmatch(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__3D9652D0_915B_46CB_98FE_8E6DAAD16817__INCLUDED_)
