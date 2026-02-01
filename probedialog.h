#if !defined(AFX_PROBEDIALOG_H__81C7A041_B152_4F5F_84FC_64D1A36D7857__INCLUDED_)
#define AFX_PROBEDIALOG_H__81C7A041_B152_4F5F_84FC_64D1A36D7857__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProbeDialog.h : header file
//

#include <algorithm>
using std::min;
using std::max;

// #include "OXLayoutManager.h"
#include "Toolkit\ot_layoutmgr.h"
#include "Grid\gxall.h"

/////////////////////////////////////////////////////////////////////////////
// CProbeGrid window

class CProbeGrid : public CGXGridWnd
{
// Construction
public:
	CProbeGrid();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProbeGrid)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProbeGrid();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProbeGrid)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CProbeDialog dialog

class CProbeDialog : public CDialog
{
// Construction
public:
	CProbeDialog();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProbeDialog)
	enum { IDD = IDD_PROBE_GRID };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

  void FillGrid(int x_index, int y_index);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProbeDialog)
	//}}AFX_VIRTUAL

// Implementation
protected:
  CProbeGrid m_wndGrid;
  // COXLayoutManager m_layoutManager;
	SECLayoutFactory m_layoutFactory;

	// Generated message map functions
	//{{AFX_MSG(CProbeDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROBEDIALOG_H__81C7A041_B152_4F5F_84FC_64D1A36D7857__INCLUDED_)
