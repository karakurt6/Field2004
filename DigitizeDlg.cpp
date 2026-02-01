// DigitizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "DigitizeDlg.h"
#include "FieldDoc.h"
#include "IntegralProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDigitizeDlg dialog


CDigitizeDlg::CDigitizeDlg()
{
	//{{AFX_DATA_INIT(CDigitizeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDigitizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDigitizeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDigitizeDlg, CDialog)
	//{{AFX_MSG_MAP(CDigitizeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDigitizeDlg message handlers

void CDigitizeDlg::OnOK() 
{
  CIntegralProp* pDlg = (CIntegralProp*) GetParent();
  pDlg->m_wndContour.SetCheck(FALSE);
	CDialog::OnOK();
}

void CDigitizeDlg::OnCancel() 
{
  CIntegralProp* pDlg = (CIntegralProp*) GetParent();
  pDlg->m_wndContour.SetCheck(FALSE);
	CDialog::OnCancel();
}
