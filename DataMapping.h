#if !defined(AFX_DATAMAPPING_H__36E073ED_2ED0_4096_96EE_69F5715EDF81__INCLUDED_)
#define AFX_DATAMAPPING_H__36E073ED_2ED0_4096_96EE_69F5715EDF81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataMapping.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDataMapping dialog

class CDataMapping : public CDialog
{
// Construction
public:
	CDataMapping(CWnd* pParent = NULL);   // standard constructor

  class Color_ListCtrl: public CListCtrl
  {
  public:
    struct DATA_COLOR { float value; COLORREF color; };
    struct DATA_TABLE { int size, count; DATA_COLOR data[1]; };

    void Initialize(DATA_TABLE* pTable, int nSelected);
    void DrawItem(LPDRAWITEMSTRUCT);
    BOOL UpdateItem(int, float);
    void InsertText(int n);

  private:
    DATA_TABLE* m_pTable;
  };

// Dialog Data
	//{{AFX_DATA(CDataMapping)
	enum { IDD = IDD_COLOR_LIST };
	Color_ListCtrl	m_ctrlList;
	//}}AFX_DATA

  enum { COLOR_MAXDATA = 256 };
  Color_ListCtrl::DATA_TABLE table;
  Color_ListCtrl::DATA_COLOR data[COLOR_MAXDATA-1];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataMapping)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDataMapping)
	virtual BOOL OnInitDialog();
	afx_msg void OnEndlabeleditList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATAMAPPING_H__36E073ED_2ED0_4096_96EE_69F5715EDF81__INCLUDED_)
