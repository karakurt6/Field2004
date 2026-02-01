#if !defined(AFX_OVERVIEW_H__B31F0908_063F_4B43_BEB1_3C0327A2BD45__INCLUDED_)
#define AFX_OVERVIEW_H__B31F0908_063F_4B43_BEB1_3C0327A2BD45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OverView.h : header file
//

class CActField;
class CFieldView;

/////////////////////////////////////////////////////////////////////////////
// COverView view

class COverView : public CView
{
protected:
	COverView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(COverView)

// Attributes
public:
	CActField* GetDocument();

  struct Color { unsigned char b, g, r; };
  Color** m_pColor;
  CBitmap m_bitmap;

  BOOL m_bCapture;
  CPoint m_pointDrag;

  // Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COverView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~COverView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(COverView)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FieldView.cpp
inline CActField* COverView::GetDocument()
   { return (CActField*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OVERVIEW_H__B31F0908_063F_4B43_BEB1_3C0327A2BD45__INCLUDED_)
