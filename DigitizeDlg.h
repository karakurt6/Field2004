#if !defined(AFX_DIGITIZEDLG_H__2EF5CB92_5197_4B16_A7A0_0822C0CF0F73__INCLUDED_)
#define AFX_DIGITIZEDLG_H__2EF5CB92_5197_4B16_A7A0_0822C0CF0F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DigitizeDlg.h : header file
//

// #include "stuff/public.h"
#include "stuff/ps_data.h"
#include <list>

class CIntegralProp;

/////////////////////////////////////////////////////////////////////////////
// CDigitizeDlg dialog

class CDigitizeDlg : public CDialog
{
// Construction
public:
	CDigitizeDlg();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDigitizeDlg)
	enum { IDD = IDD_DIGITIZE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

  typedef std::list<coord_type> Coord_list;
  Coord_list data;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDigitizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CIntegralProp* GetProp() const;

	// Generated message map functions
	//{{AFX_MSG(CDigitizeDlg)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIGITIZEDLG_H__2EF5CB92_5197_4B16_A7A0_0822C0CF0F73__INCLUDED_)
