// FieldView.h : interface of the CFieldView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FIELDVIEW_H__01493B01_4411_4A75_8CE7_DC55794852DA__INCLUDED_)
#define AFX_FIELDVIEW_H__01493B01_4411_4A75_8CE7_DC55794852DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include "stuff\public.h"
#include "stuff\ps_data.h"

class CActField;
class COverView;
class CFieldView;

/////////////////////////////////////////////////////////////////////////////
// CColorPalette window

class CColorPalette : public CWnd
{
// Construction
public:
	CColorPalette();

// Attributes
public:

// Operations
public:
  BOOL CreateWnd(CFieldView* pParentWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorPalette)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorPalette();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorPalette)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

class CFieldView : public CScrollView
{
protected: // create from serialization only
	CFieldView();
	DECLARE_DYNCREATE(CFieldView)

// Attributes
public:
	CActField* GetDocument();

  enum GridType { GRID_NONE, GRID_CELLS, GRID_NODES };
  GridType m_gridType;

  enum DataType { DATA_CELLS, DATA_NODES, DATA_LEVELS };
  DataType m_dataType;

  BOOL m_bColoring;
  CColorPalette m_wndColorPalette;

// Operations
public:
  void DoOverview();
  void SyncWellList(CPoint point);
  void AppendCtrlPoint(CPoint point);
  void AppendCtrlPoint(coord_type* p);
  void UpdateCtrlPoint(int);
  void UpdateCrossSection();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFieldView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFieldView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  BOOL m_bScale;

  enum HitType { HIT_NONE, HIT_POINT, HIT_SEGMENT };
  HitType m_enumHit;
  int m_nHitTest;
  static UINT s_nDragPointFormat;
  COleDropTarget m_cmdDropTarget;
  void HitCrossSection(CPoint);

	long m_nBoreSelected;

// Generated message map functions
protected:
	//{{AFX_MSG(CFieldView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnFieldViewProp();
	afx_msg void OnFieldScale();
	afx_msg void OnFieldColoring();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnFieldQuerywellbore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnFieldStart3dviewer();
	afx_msg void OnInclinometry();
};

#ifndef _DEBUG  // debug version in FieldView.cpp
inline CActField* CFieldView::GetDocument()
   { return (CActField*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIELDVIEW_H__01493B01_4411_4A75_8CE7_DC55794852DA__INCLUDED_)
