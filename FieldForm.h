#if !defined(AFX_FIELDFORM_H__A34C864C_23FF_4858_99BE_8ECF6020978E__INCLUDED_)
#define AFX_FIELDFORM_H__A34C864C_23FF_4858_99BE_8ECF6020978E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FieldForm.h : header file
//

#include <list>
#include <map>
// #include "stuff/public.h"
#include "stuff\ps_data.h"

class CActField;

/////////////////////////////////////////////////////////////////////////////
// CFieldForm form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class CFieldForm : public CFormView
{
protected:
	CFieldForm();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFieldForm)

// Form Data
public:
	//{{AFX_DATA(CFieldForm)
	enum { IDD = IDD_FIELD_FORM };
	CComboBox	m_ctrlProp;
	CSpinButtonCtrl	m_ctrlSpin;
	CSliderCtrl	m_ctrlBed;
	CListCtrl	m_ctrlExt;
	CListCtrl	m_ctrlInj;
	CMonthCalCtrl	m_ctrlCal;
	int		m_nBed;
	//}}AFX_DATA

// Attributes
public:
  CActField* GetDocument();

  struct ext_data_struct
  {
    char well[8];
    char layer[8];
    long bore;
    float oil, wat, gas;
    float loc[3];
  };
  typedef std::list<ext_data_struct> ext_data_list;

  struct inj_data_struct
  {
    char well[8];
    char layer[8];
    long bore;
    float fluid;
    float loc[3];
  };
  typedef std::list<inj_data_struct> inj_data_list;

  COleDateTime m_dateStart, m_dateFinal, m_dateSel;
  ext_data_list m_listExt;
  inj_data_list m_listInj;

// Operations
public:
  void Populate();
  void InitPropList();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFieldForm)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFieldForm();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CFieldForm)
	afx_msg void OnSelchangeMonthcalendar1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectMonthcalendar1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickExtlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickInjlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombo1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	struct Well
	{
		long wellbore;
		char wellname[16];
		char formation[64];
		char opcode[64];
		DATE start;
		DATE final;
		float oil;
		float water;
		float gas;
		float fluid;
		float puregas;

		struct Status
		{
			DBSTATUS oil;
			DBSTATUS water;
			DBSTATUS gas;
			DBSTATUS fluid;
			DBSTATUS puregas;
		};

		Status status;

		BEGIN_COLUMN_MAP(Well)
			COLUMN_ENTRY(1, wellbore)
			COLUMN_ENTRY(2, wellname)
			COLUMN_ENTRY(3, formation)
			COLUMN_ENTRY(4, opcode)
			COLUMN_ENTRY(5, start)
			COLUMN_ENTRY(6, final)
			COLUMN_ENTRY_STATUS(7, oil, status.oil)
			COLUMN_ENTRY_STATUS(8, water, status.water)
			COLUMN_ENTRY_STATUS(9, gas, status.gas)
			COLUMN_ENTRY_STATUS(10, fluid, status.fluid)
			COLUMN_ENTRY_STATUS(11, puregas, status.puregas)
		END_COLUMN_MAP()
	};

	struct Date
	{
		DATE startdate;
		DATE finaldate;
		BEGIN_COLUMN_MAP(Date)
			COLUMN_ENTRY(1, startdate)
			COLUMN_ENTRY(2, finaldate)
		END_COLUMN_MAP()
	};
};

#ifndef _DEBUG  // debug version in FieldView.cpp
inline CActField* CFieldForm::GetDocument()
   { return (CActField*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIELDFORM_H__A34C864C_23FF_4858_99BE_8ECF6020978E__INCLUDED_)
