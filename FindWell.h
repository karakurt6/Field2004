#if !defined(AFX_FINDWELL_H__A4B3B207_FBB1_4871_AC6E_F05BCD25BA6A__INCLUDED_)
#define AFX_FINDWELL_H__A4B3B207_FBB1_4871_AC6E_F05BCD25BA6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindWell.h : header file
//

#include <list>
// #include "OXLayoutManager.h"
#include "Toolkit\ot_layoutmgr.h"
#include "atldbcli.h"

/////////////////////////////////////////////////////////////////////////////
// CFindWell dialog

class CFindWell : public CDialog
{
// Construction
public:
	CFindWell(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindWell)
	enum { IDD = IDD_FINDWELL };
	CListBox	m_wndList;
	CEdit	m_wndEdit;
	int		m_nSort;
	//}}AFX_DATA

  struct Well
  {
    long bore;
    char name[16];
    float x, y;

		BEGIN_COLUMN_MAP(Well)
			COLUMN_ENTRY(2, bore)
			COLUMN_ENTRY(1, name)
		END_COLUMN_MAP()
  };

  std::list<Well> m_listWell;

	void PopulateList();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindWell)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  // COXLayoutManager m_layoutManager;
	SECLayoutFactory m_layoutFactory;

	// Generated message map functions
	//{{AFX_MSG(CFindWell)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnChangeEdit1();
	afx_msg void OnClose();
	afx_msg void OnDblclkList1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
  void OnOK();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDWELL_H__A4B3B207_FBB1_4871_AC6E_F05BCD25BA6A__INCLUDED_)
