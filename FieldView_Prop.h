#if !defined(AFX_FIELDVIEW_PROP_H__FBAA865E_6118_4F6D_98CD_19AF586EAA53__INCLUDED_)
#define AFX_FIELDVIEW_PROP_H__FBAA865E_6118_4F6D_98CD_19AF586EAA53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FieldView_Prop.h : header file
//

class CFieldView;

/////////////////////////////////////////////////////////////////////////////
// CFieldView_PageGrid dialog

class CFieldView_PageGrid : public CPropertyPage
{
	DECLARE_DYNCREATE(CFieldView_PageGrid)

// Construction
public:
	CFieldView_PageGrid();
	~CFieldView_PageGrid();

// Dialog Data
	//{{AFX_DATA(CFieldView_PageGrid)
	enum { IDD = IDD_FIELD_GRID };
	int		m_typePage;
	//}}AFX_DATA

  int m_typeInit;
  CFieldView* m_pView;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFieldView_PageGrid)
	public:
	virtual void OnOK();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFieldView_PageGrid)
	afx_msg void OnPageChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CFieldView_PageData dialog

class CFieldView_PageData : public CPropertyPage
{
	DECLARE_DYNCREATE(CFieldView_PageData)

// Construction
public:
	CFieldView_PageData();
	~CFieldView_PageData();

// Dialog Data
	//{{AFX_DATA(CFieldView_PageData)
	enum { IDD = IDD_FIELD_DATA };
	int		m_typePage;
	//}}AFX_DATA

  int m_typeInit;
  CFieldView* m_pView;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFieldView_PageData)
	public:
	virtual void OnOK();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CFieldView_PageData)
	afx_msg void OnPageChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CFieldView_Prop

class CFieldView_Prop : public CPropertySheet
{
	DECLARE_DYNAMIC(CFieldView_Prop)

// Construction
public:
  CFieldView_Prop(CFieldView* pParentWnd);

// Attributes
public:
  CFieldView_PageGrid m_pageGrid;
  CFieldView_PageData m_pageData;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFieldView_Prop)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFieldView_Prop();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFieldView_Prop)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIELDVIEW_PROP_H__FBAA865E_6118_4F6D_98CD_19AF586EAA53__INCLUDED_)
