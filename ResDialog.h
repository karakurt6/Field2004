#if !defined(AFX_RESDIALOG_H__31BE6926_ED1B_40B6_AF4C_EBC7FDFFF658__INCLUDED_)
#define AFX_RESDIALOG_H__31BE6926_ED1B_40B6_AF4C_EBC7FDFFF658__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResDialog dialog

#include <list>
#include <valarray>
using std::min;
using std::max;
#include <Grid\gxall.h>
#include "FieldDoc.h"

class CResGrid: public CGXGridWnd
{
public:
  virtual BOOL SetCurrentCell(ROWCOL nRow, ROWCOL nCol, UINT flags = GX_SCROLLINVIEW | GX_UPDATENOW);
};

class CResDialog : public CDialog
{
// Construction
public:
	CResDialog();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResDialog)
	enum { IDD = IDD_RESERVES };
	float	m_sOilDensity;
	float	m_sShrinkingFactor;
	float	m_sBaricCoef;
	float	m_sThermalFactor;
	//}}AFX_DATA

  struct DataItem 
  {
    char name[8];
    char top[80];
    char base[80];
    float scale_o;
    float scale_g;
    float* oil; 
    float* gas; 
  };

  typedef std::list<DataItem*> DataList;

  CActField::Palette* m_pOilTabl;
  CActField::Palette* m_pGasTabl;

  int m_nOilCol;
  int m_nGasCol;
  CResGrid m_wndGrid;
  DataList m_listData;
  int m_nRowSel;
  int m_nColSel;

  BOOL IsAvailable() const;
  void RecalcData();
  void SetCurrentCell(int nRow, int nCol);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CActField* m_pDoc;
  void Initialize(CActField* pDoc);

	// Generated message map functions
	//{{AFX_MSG(CResDialog)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnEdit();
	afx_msg void OnDestroy();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESDIALOG_H__31BE6926_ED1B_40B6_AF4C_EBC7FDFFF658__INCLUDED_)
