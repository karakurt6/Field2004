#if !defined(AFX_CUMULATIVES_H__6760B412_27CC_4AC2_80E7_04B931606E04__INCLUDED_)
#define AFX_CUMULATIVES_H__6760B412_27CC_4AC2_80E7_04B931606E04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CumulativeS.h : header file
//

#include <libh5/hdf5.h>
#include <Toolkit\ot_colorwell.h>

class CActField;
struct color_type;

/////////////////////////////////////////////////////////////////////////////
// CCumulativeS dialog

class CCumulativeS : public CDialog
{
// Construction
public:
	CCumulativeS();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCumulativeS)
	enum { IDD = IDD_SATURATION };
		// NOTE: the ClassWizard will add data members here
		// number segments on each side
		int m_nSize;
	//}}AFX_DATA

  hid_t id_o;
  hid_t id_g;
	hid_t id_c;
  float *fn_o;
  float *fn_g;
	float *fn_c;
  bool init;

	SECWellButton m_minColor;
	SECWellButton m_maxColor;
	float m_minCollector;
	float m_maxCollector;
	color_type *m_pColorData;

	CActField* GetDocument() const;
  bool IsAvailable() const;
	void LookupColor(double o, double g, double c, color_type* s) const;
  void OnHideWindow();
  void UpdateSelection();
  void DrawDataNodes(CDC* pDC, const int* pg, const int* vp);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCumulativeS)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
  BOOL m_bColorPalette;
	CSpinButtonCtrl m_wndSpinButton;
public:
	virtual ~CCumulativeS();

	// Generated message map functions
	//{{AFX_MSG(CCumulativeS)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUMULATIVES_H__6760B412_27CC_4AC2_80E7_04B931606E04__INCLUDED_)
