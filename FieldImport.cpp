// FieldImport.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "FieldImport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFieldImport dialog


CFieldImport::CFieldImport(CWnd* pParent /*=NULL*/)
	: CDialog(CFieldImport::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFieldImport)
	geological_database = _T("");
	layer_thickness = 0.0f;
	depth_to_top = _T("");
	min_value = 0.0f;
	max_value = 0.0f;
	property = _T("");
	num_layers = 0;
	data_mapping = -1;
	//}}AFX_DATA_INIT
  open_flag = false;
}


void CFieldImport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFieldImport)
	DDX_Text(pDX, IDC_EDIT2, geological_database);
	DDX_Text(pDX, IDC_EDIT3, layer_thickness);
	DDX_Text(pDX, IDC_EDIT1, depth_to_top);
	DDX_Text(pDX, IDC_EDIT10, min_value);
	DDX_Text(pDX, IDC_EDIT11, max_value);
	DDX_Text(pDX, IDC_EDIT4, property);
	DDX_Text(pDX, IDC_EDIT5, num_layers);
	DDX_Radio(pDX, IDC_RADIO1, data_mapping);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFieldImport, CDialog)
	//{{AFX_MSG_MAP(CFieldImport)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldImport message handlers

BOOL CFieldImport::OnInitDialog() 
{
	CDialog::OnInitDialog();

  if (open_flag)
  {
    GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT3)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT5)->EnableWindow(FALSE);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFieldImport::OnButton1() // depth-to-top
{
  CFileDialog dlg(TRUE, ".grd", NULL, OFN_PATHMUSTEXIST, "Surfer grid files (*.grd)|*.grd|"
    "All files (*.*)|*.*||", this);

  if (dlg.DoModal() == IDOK)
  {
    GetDlgItem(IDC_EDIT1)->SetWindowText(dlg.GetPathName());
  }
}

void CFieldImport::OnButton2() // database
{
  CFileDialog dlg(TRUE, ".mdb", NULL, OFN_PATHMUSTEXIST, "Microsoft Jet database (*.mdb)|*.mdb|"
    "All files (*.*)|*.*||", this);

  if (dlg.DoModal() == IDOK)
  {
    GetDlgItem(IDC_EDIT2)->SetWindowText(dlg.GetPathName());
  }
}

void CFieldImport::OnButton3() // archive
{
  CFileDialog dlg(TRUE, ".arz", NULL, OFN_PATHMUSTEXIST, "Compressed archive (*.arz)|*.arz|"
    "All files (*.*)|*.*||", this);

  if (dlg.DoModal() == IDOK)
  {
    GetDlgItem(IDC_EDIT4)->SetWindowText(dlg.GetPathName());
  }
}
