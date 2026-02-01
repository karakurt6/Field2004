#if !defined(AFX_INTEGRALPROP_H__46D175C6_692A_42B3_B2FB_0E6B625FD73A__INCLUDED_)
#define AFX_INTEGRALPROP_H__46D175C6_692A_42B3_B2FB_0E6B625FD73A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntegralProp.h : header file
//

// #include <OxBrowseDirEdit.h>
#include "DigitizeDlg.h"
#include <Toolkit\atl\otlbase.h>
#include <Toolkit\browedit.h>

/////////////////////////////////////////////////////////////////////////////
// CIntegralProp dialog

class IntegralProp_Impl;

class CIntegralProp : public CDialog
{
// Construction
public:
  CIntegralProp();   // standard constructor
  ~CIntegralProp();

// Dialog Data
  //{{AFX_DATA(CIntegralProp)
	enum { IDD = IDD_FIELD_EXPR };
	CButton	m_wndContour;
	CListCtrl	m_wndList1;
	SECBrowseFileEdit	m_wndEdit8;
	CEdit	m_wndEdit7;
	SECBrowseFileEdit	m_wndEdit6;
	SECBrowseFileEdit	m_wndEdit5;
	CEdit	m_wndEdit4;
	CEdit	m_wndEdit3;
	CEdit	m_wndEdit2;
	CEdit	m_wndEdit1;
	//}}AFX_DATA

  CDigitizeDlg m_wndDigitize;
  BOOL m_bActive;
  void SelectItem(int nItem);
  float* GetData();
  CActField::Palette* GetPalette();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CIntegralProp)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CIntegralProp)
	afx_msg void OnUpdate();
	afx_msg void OnRemove();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg void OnDigitize();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
  virtual void OnCancel();

private:
  IntegralProp_Impl* m_pImpl;
  CImageList m_imgState;
  int m_nItem;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTEGRALPROP_H__46D175C6_692A_42B3_B2FB_0E6B625FD73A__INCLUDED_)
