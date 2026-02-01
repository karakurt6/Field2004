// PlotDialog.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "PlotDialog.h"
#include "plotdialog.h"
#include "stuff\ps_font.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlotDialog dialog


CPlotDialog::CPlotDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPlotDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlotDialog)
	format = -1;
	left_margin = 0;
	font_size = 0;
	font_family = _T("");
	num_pages = 0;
	right_margin = 0;
	top_margin = 0;
	bottom_margin = 0;
	x_major_tick = 0;
	x_minor_tick = 0;
	y_major_tick = 0;
	y_minor_tick = 0;
	orientation = -1;
	show_grid = FALSE;
	show_legend = FALSE;
	page_numbering = FALSE;
	show_formation = FALSE;
	//}}AFX_DATA_INIT
}


void CPlotDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlotDialog)
	DDX_CBIndex(pDX, IDC_COMBO1, format);
	DDX_Text(pDX, IDC_EDIT1, left_margin);
	DDX_Text(pDX, IDC_EDIT12, font_size);
	DDX_Text(pDX, IDC_EDIT2, num_pages);
	DDX_Text(pDX, IDC_EDIT3, right_margin);
	DDX_Text(pDX, IDC_EDIT4, top_margin);
	DDX_Text(pDX, IDC_EDIT5, bottom_margin);
	DDX_Text(pDX, IDC_EDIT6, x_major_tick);
	DDX_Text(pDX, IDC_EDIT7, x_minor_tick);
	DDX_Text(pDX, IDC_EDIT8, y_major_tick);
	DDX_Text(pDX, IDC_EDIT9, y_minor_tick);
	DDX_Radio(pDX, IDC_RADIO1, orientation);
	DDX_Check(pDX, IDC_CHECK1, show_grid);
	DDX_Check(pDX, IDC_CHECK2, show_legend);
	DDX_Check(pDX, IDC_CHECK3, page_numbering);
	DDX_Text(pDX, IDC_EDIT10, description);
	DDX_CBString(pDX, IDC_COMBO3, font_family);
	DDX_Check(pDX, IDC_CHECK4, show_formation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlotDialog, CDialog)
	//{{AFX_MSG_MAP(CPlotDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
//	ON_BN_CLICKED(IDC_CHECK4, OnBnClickedCheck4)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotDialog message handlers

BOOL CPlotDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CComboBox* pWnd = (CComboBox*) GetDlgItem(IDC_COMBO3);
	pWnd->AddString(FONT_01);
	pWnd->AddString(FONT_02);
	pWnd->AddString(FONT_03);
	pWnd->AddString(FONT_04);
	pWnd->AddString(FONT_05);
	pWnd->AddString(FONT_06);
	pWnd->AddString(FONT_07);
	pWnd->AddString(FONT_08);
	pWnd->AddString(FONT_09);
	pWnd->AddString(FONT_10);
	pWnd->AddString(FONT_11);
	pWnd->AddString(FONT_12);
	pWnd->AddString(FONT_13);
	pWnd->AddString(FONT_14);
	pWnd->AddString(FONT_15);
	pWnd->AddString(FONT_16);
	pWnd->AddString(FONT_17);
	pWnd->AddString(FONT_18);
	pWnd->AddString(FONT_19);
	pWnd->AddString(FONT_20);
	pWnd->AddString(FONT_21);
	pWnd->AddString(FONT_22);
	pWnd->AddString(FONT_23);
	pWnd->AddString(FONT_24);
	pWnd->AddString(FONT_25);
	pWnd->AddString(FONT_26);
	pWnd->AddString(FONT_27);
	pWnd->AddString(FONT_28);
	pWnd->AddString(FONT_29);
	pWnd->AddString(FONT_30);
	pWnd->AddString(FONT_31);
	pWnd->AddString(FONT_32);
	pWnd->AddString(FONT_33);
	pWnd->AddString(FONT_34);
	pWnd->AddString(FONT_35);
	pWnd->AddString(FONT_36);
	pWnd->AddString(FONT_37);
	pWnd->AddString(FONT_38);
	pWnd->AddString(FONT_39);
	pWnd->AddString(FONT_40);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

