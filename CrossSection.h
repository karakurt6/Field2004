#if !defined(AFX_CROSSSECTION_H__72519C14_EC1B_4989_9A58_5896C5F2ACB2__INCLUDED_)
#define AFX_CROSSSECTION_H__72519C14_EC1B_4989_9A58_5896C5F2ACB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CrossSection.h : header file
//

#include <Toolkit\atl\otlbase.h>
#include <Toolkit\browedit.h>

/////////////////////////////////////////////////////////////////////////////
// CCrossSection dialog

class CCrossSection : public CDialog
{
// Construction
public:
	CCrossSection();
  BOOL m_bVisible;

  enum Format { FORMAT_A4, FORMAT_A3, FORMAT_A0 };
  enum Orientation { LANDSCAPE, PORTRAIT };
  enum DataMapping { ARITHMETIC, LOGARITHMIC };

  DataMapping data_mapping;
  int x_major_tick;
  int x_minor_tick;
  int y_major_tick;
  int y_minor_tick;
  bool show_grid;
  bool show_legend;
  int left_margin;
  int right_margin;
  int top_margin;
  int bottom_margin;
  int num_pages;
  char font_family[80];
  int font_size;
  Format format;
  Orientation orientation;
  int page_width;
  int page_height;
	bool page_numbering;
	CString description;
	bool show_formation;

// Dialog Data
	//{{AFX_DATA(CCrossSection)
	enum { IDD = IDD_CROSS_SECTION };
	SECBrowseFileEdit	m_ctrlApp;
	SECBrowseFileEdit	m_ctrlPlot;
	CListCtrl	m_ctrlPoints;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrossSection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  BOOL m_bRun;

  void Generate(const char* name);
  void InitPageSizes();

	// Generated message map functions
	//{{AFX_MSG(CCrossSection)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnCheck1();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlotCrossSection();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnPlotProperties();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CROSSSECTION_H__72519C14_EC1B_4989_9A58_5896C5F2ACB2__INCLUDED_)
