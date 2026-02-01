#if !defined(AFX_PLOTDIALOG_H__12B13D9C_C57A_435E_AC71_2D0649384AFB__INCLUDED_)
#define AFX_PLOTDIALOG_H__12B13D9C_C57A_435E_AC71_2D0649384AFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlotDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlotDialog dialog

class CPlotDialog : public CDialog
{
// Construction
public:
	CPlotDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlotDialog)
	enum { IDD = IDD_PLOT_PROPERTIES };
	int		format;
	UINT	left_margin;
	UINT	font_size;
	CString	font_family;
	UINT	num_pages;
	UINT	right_margin;
	UINT	top_margin;
	UINT	bottom_margin;
	UINT	x_major_tick;
	UINT	x_minor_tick;
	UINT	y_major_tick;
	UINT	y_minor_tick;
	int		orientation;
	BOOL	show_grid;
	BOOL	show_legend;
	BOOL page_numbering;
	CString description;
	BOOL show_formation;
	CString font_name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlotDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlotDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLOTDIALOG_H__12B13D9C_C57A_435E_AC71_2D0649384AFB__INCLUDED_)
