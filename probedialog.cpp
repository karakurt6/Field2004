// ProbeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "ProbeDialog.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include <valarray>
#include <libh5/hdf5.h>

#pragma warning(disable: 4786)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProbeGrid

CProbeGrid::CProbeGrid()
{
}

CProbeGrid::~CProbeGrid()
{
}


BEGIN_MESSAGE_MAP(CProbeGrid, CGXGridWnd)
	//{{AFX_MSG_MAP(CProbeGrid)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProbeGrid message handlers


/////////////////////////////////////////////////////////////////////////////
// CProbeDialog dialog


CProbeDialog::CProbeDialog(): CDialog(CProbeDialog::IDD, NULL)
{
	//{{AFX_DATA_INIT(CProbeDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


/*
void CProbeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProbeDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

}
*/

BEGIN_MESSAGE_MAP(CProbeDialog, CDialog)
	//{{AFX_MSG_MAP(CProbeDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProbeDialog message handlers

#define OX_OFFSET (7)

BOOL CProbeDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  // Please refer to the MFC documentation on SubclassDlgItem for 
  // information on this call. This makes sure that our C++ grid 
  // window class subclasses the window that is created with the 
  // User Control.
  m_wndGrid.SubclassDlgItem(IDC_CUSTOM1, this);

  // Initialize the grid. For CWnd based grids this call is 
  // essential. For view based grids this initialization is done 
  // in OnInitialUpdate.
  m_wndGrid.Initialize();

  m_wndGrid.GetParam()->EnableUndo(FALSE);
  m_wndGrid.SetReadOnly();

  m_wndGrid.GetParam()->EnableUndo(TRUE);
  // m_wndGrid.GetParam()->SetNewGridLineMode(TRUE);
  // m_wndGrid.GetParam()->EnableSelection(GX_SELNONE);
  m_wndGrid.GetParam()->EnableThumbTrack(TRUE);

	CGXProperties* pProp = m_wndGrid.GetParam()->GetProperties();
	pProp->SetColor(GX_COLOR_BACKGROUND, GetSysColor(COLOR_BTNFACE));

	/*
  m_layoutManager.Attach(this);
  m_layoutManager.SetConstraint(IDOK, OX_LMS_BOTTOM | OX_LMS_RIGHT, OX_LMT_SAME, -OX_OFFSET);
  m_layoutManager.SetConstraint(IDC_CUSTOM1, OX_LMS_BOTTOM, OX_LMT_OPPOSITE, -OX_OFFSET, IDOK);
  m_layoutManager.SetConstraint(IDC_CUSTOM1, OX_LMS_RIGHT, OX_LMT_SAME, -OX_OFFSET);
  m_layoutManager.SetConstraint(IDC_CUSTOM1, OX_LMS_LEFT | OX_LMS_TOP, OX_LMT_SAME, OX_OFFSET);
	*/

	SECLNRelative* pRelNode = (SECLNRelative*) m_layoutFactory.CreateNode(NODE_CLASS(SECLNRelative));
	SECLayoutNodeWnd* pButton = m_layoutFactory.CreateNodeWnd(IDOK, this, pRelNode);
	SECLayoutNodeWnd* pCustom = m_layoutFactory.CreateNodeWnd(IDC_CUSTOM1, this, pRelNode);

	pRelNode->SetConstraint(pButton, REL_MOVEB, pRelNode, REL_BOTTOM, -OX_OFFSET);
	pRelNode->SetConstraint(pButton, REL_MOVER, pRelNode, REL_RIGHT, -OX_OFFSET);
	pRelNode->SetConstraint(pCustom, REL_BOTTOM, pButton, REL_TOP, -OX_OFFSET);
	pRelNode->SetConstraint(pCustom, REL_RIGHT, pRelNode, REL_RIGHT, -OX_OFFSET);

	// Set the window listener
	SECLayoutWndListener* pListener = m_layoutFactory.CreateLayoutWndListener();
	pListener->AutoInit(pRelNode, this);

	return TRUE;  // return TRUE unless you set the focus to a control
									// EXCEPTION: OCX Property Pages should return FALSE
}

void CProbeDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
  if (bShow)
  {
    CActField* pDoc = GetFieldView()->GetDocument();
    const int max_p = 20;
    const CActField::DataField* pp[max_p];
    int nn = pDoc->FieldEnum(max_p, pp);
    ASSERT(nn <= max_p);
    int n = m_wndGrid.GetColCount();
    if (n != nn)
    {
      m_wndGrid.GetParam()->SetLockReadOnly(FALSE);
      BOOL bOldLock = m_wndGrid.LockUpdate(TRUE);
      m_wndGrid.SetColCount(nn);
      m_wndGrid.SetRowCount(pDoc->cz);
      for (int i = 1; i <= nn; ++i)
      {
        m_wndGrid.SetStyleRange(CGXRange(0, i), CGXStyle().SetValue(pp[i-1]->name));
      }
      m_wndGrid.LockUpdate(bOldLock);
      m_wndGrid.GetParam()->SetLockReadOnly(TRUE);
      FillGrid(0, 0);
    }
  }
	CDialog::OnShowWindow(bShow, nStatus);	
}

void CProbeDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
  if (cx != 0 && cy != 0)
  {
    if (m_wndGrid.GetParam() != 0)
      m_wndGrid.Redraw();
  }
}

void CProbeDialog::FillGrid(int x_index, int y_index)
{
  ASSERT(IsWindow(m_wndGrid));
  
  CWaitCursor wait;
  m_wndGrid.GetParam()->SetLockReadOnly(FALSE);
  BOOL bOldLock = m_wndGrid.LockUpdate(TRUE);

  CFieldView* pView = GetFieldView();
  CActField* pDoc = pView->GetDocument();
  const int max_p = 20;
  const CActField::DataField* pp[max_p];
  int nn = pDoc->FieldEnum(max_p, pp);
  ASSERT(nn <= max_p);
  std::valarray<float> zz(pDoc->cz);
  for (int i = 0; i < nn; ++i)
  {
    hid_t fd = pp[i]->id;
    hid_t fs = H5Dget_space(fd);
    hsize_t start[3] = { 0, y_index, x_index };
    hsize_t count[3] = { pDoc->cz, 1, 1 };
    herr_t err = H5Sselect_hyperslab(fs, H5S_SELECT_SET, start, 0, count, 0);
    hid_t ms = H5Screate_simple(1, count, 0);
    err = H5Dread(fd, H5T_NATIVE_FLOAT, ms, fs, H5P_DEFAULT, &zz[0]);
    err = H5Sclose(ms);
    err = H5Sclose(fs);
    for (int nRow = 1, nCol = i+1; nRow <= pDoc->cz; ++nRow)
    {
      m_wndGrid.SetValueRange(CGXRange(nRow, nCol), zz[nRow-1]);
    }
  }
  m_wndGrid.LockUpdate(bOldLock);
  m_wndGrid.GetParam()->SetLockReadOnly(TRUE);
  m_wndGrid.Redraw();

  CString title;
  title.Format("Cell (%d, %d) -- Probe", x_index, y_index);
  SetWindowText(title);
}
