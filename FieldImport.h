#if !defined(AFX_FIELDIMPORT_H__A52D469D_164E_4F8B_A778_324002D5F643__INCLUDED_)
#define AFX_FIELDIMPORT_H__A52D469D_164E_4F8B_A778_324002D5F643__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FieldImport.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFieldImport dialog

class CFieldImport : public CDialog
{
// Construction
public:
	CFieldImport(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFieldImport)
	enum { IDD = IDD_NEW_DOCUMENT };
	CString	geological_database;
	float	layer_thickness;
	CString	depth_to_top;
	float	min_value;
	float	max_value;
	CString	property;
	int		num_layers;
	int		data_mapping;
	//}}AFX_DATA
  bool open_flag;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFieldImport)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFieldImport)
	virtual BOOL OnInitDialog();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIELDIMPORT_H__A52D469D_164E_4F8B_A778_324002D5F643__INCLUDED_)
