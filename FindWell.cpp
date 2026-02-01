// FindWell.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "FindWell.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "FieldForm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindWell dialog


CFindWell::CFindWell(CWnd* pParent /*=NULL*/)
	: CDialog(CFindWell::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindWell)
	m_nSort = 0;
	//}}AFX_DATA_INIT
}


void CFindWell::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindWell)
	DDX_Control(pDX, IDC_LIST1, m_wndList);
	DDX_Control(pDX, IDC_EDIT1, m_wndEdit);
	DDX_Radio(pDX, IDC_RADIO1, m_nSort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindWell, CDialog)
	//{{AFX_MSG_MAP(CFindWell)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_WM_CLOSE()
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindWell message handlers

#define OX_OFFSET (7)

BOOL CFindWell::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	/*
  m_layoutManager.Attach(this);
  m_layoutManager.SetConstraint(IDC_EDIT1, OX_LMS_TOP, OX_LMT_OPPOSITE, OX_OFFSET, IDC_RADIO2);
  m_layoutManager.SetConstraint(IDC_EDIT1, OX_LMS_LEFT, OX_LMT_SAME, OX_OFFSET);
  m_layoutManager.SetConstraint(IDC_EDIT1, OX_LMS_RIGHT, OX_LMT_SAME, -OX_OFFSET);
  m_layoutManager.SetConstraint(IDC_LIST1, OX_LMS_LEFT, OX_LMT_SAME, OX_OFFSET);
  m_layoutManager.SetConstraint(IDC_LIST1, OX_LMS_RIGHT | OX_LMS_BOTTOM, OX_LMT_SAME, -OX_OFFSET);
  m_layoutManager.SetConstraint(IDC_LIST1, OX_LMS_TOP, OX_LMT_OPPOSITE, OX_OFFSET, IDC_EDIT1);
	*/

	// create top-level relative layout node
	SECLNRelative* pRelNode = (SECLNRelative*) m_layoutFactory.CreateNode(NODE_CLASS(SECLNRelative));
	// SECLayoutNodeWnd* pRadio1Node = m_layoutFactory.CreateNodeWnd(IDC_RADIO1, this, pRelNode);
	// SECLayoutNodeWnd* pRadio2Node = m_layoutFactory.CreateNodeWnd(IDC_RADIO2, this, pRelNode);
	SECLayoutNodeWnd* pEdit1Node = m_layoutFactory.CreateNodeWnd(IDC_EDIT1, this, pRelNode);
	SECLayoutNodeWnd* pList1Node = m_layoutFactory.CreateNodeWnd(IDC_LIST1, this, pRelNode);

	pRelNode->SetConstraint(pList1Node, REL_BOTTOM, pRelNode, REL_BOTTOM, -OX_OFFSET);
	pRelNode->SetConstraint(pList1Node, REL_RIGHT, pRelNode, REL_RIGHT, -OX_OFFSET);
	pRelNode->SetConstraint(pEdit1Node, REL_RIGHT, pRelNode, REL_RIGHT, -OX_OFFSET);

	SECLayoutWndListener* pListener = m_layoutFactory.CreateLayoutWndListener();
	pListener->AutoInit(pRelNode, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindWell::PopulateList()
{
	if (!m_listWell.empty())
	{
		m_listWell.clear();
	}

  CWaitCursor wait;

  CActField* pDoc = GetFieldView()->GetDocument();
  ASSERT_VALID(pDoc);

	CTable< CAccessor<Well> > item;
	HRESULT hr = item.Open(pDoc->session, "well");
	if (SUCCEEDED(hr))
	{
		while (item.MoveNext() == S_OK)
		{
			double pp[3];
#if 0
			if (item.bore == 10000136)
			{
				DebugBreak();
			}
#endif
			if (0 != pDoc->Traj(item.bore, 1, pp))
			{
				item.x = (float) pp[0];
				item.y = (float) pp[1];
				m_listWell.push_back(item);
			}
		}
		if (m_nSort)
		{
			OnRadio2();
		}
		else
		{
			OnRadio1();
		}
	}
}

void CFindWell::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

  if (bShow)
  {
    if (m_listWell.empty())
    {
			PopulateList();
    }
  }
	CDialog::OnShowWindow(bShow, nStatus);	  

  // CFieldView* pView = GetFieldView();
  // pView->m_wndColorPalette.Invalidate();
  // pView->Invalidate();
  GetFieldView()->GetDocument()->UpdateAllViews(0, IDD);
}

void CFindWell::OnRadio1() 
{
  m_wndList.SetRedraw(FALSE);
  m_wndList.ResetContent();
  for (std::list<Well>::iterator it = m_listWell.begin(); it != m_listWell.end(); ++it)
  {
    Well& item = *it;
    CString str;
    str.Format("%s (%d)", item.name, item.bore);
    int n = m_wndList.AddString(str);
    m_wndList.SetItemData(n, (DWORD) &item);
  }
  m_wndList.SetRedraw(TRUE);
}

void CFindWell::OnRadio2() 
{
  m_wndList.SetRedraw(FALSE);
  m_wndList.ResetContent();
  for (std::list<Well>::iterator it = m_listWell.begin(); it != m_listWell.end(); ++it)
  {
    Well& item = *it;
    CString str;
    str.Format("%d (%s)", item.bore, item.name);
    int n = m_wndList.AddString(str);
    m_wndList.SetItemData(n, (DWORD) &item);
  }
  m_wndList.SetRedraw(TRUE);
}


void CFindWell::OnChangeEdit1() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
  CString str;
  m_wndEdit.GetWindowText(str);
  m_wndList.SelectString(-1, str);
}

void CFindWell::OnClose() 
{
  GetFieldView()->GetDocument()->UpdateAllViews(0, IDD);
	CDialog::OnClose();
}

void CFindWell::OnDblclkList1() 
{
  int n = m_wndList.GetCurSel();
  if (n != LB_ERR)
  {
    Well* well = (Well*) m_wndList.GetItemData(n);

    CFieldView *pFieldView = GetFieldView();
    CSize sizeTotal = pFieldView->GetTotalSize();
    CActField* pDoc = pFieldView->GetDocument();
    int x = int(sizeTotal.cx * (well->x - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
    int y = int(sizeTotal.cy * (well->y - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);

    CRect rectClient;
    pFieldView->GetClientRect(&rectClient);
    CClientDC dc(pFieldView);
    pFieldView->OnPrepareDC(&dc);
    dc.DPtoLP(&rectClient);
    CPoint pointCenter = rectClient.CenterPoint();
    CSize sizeScroll = pointCenter - CPoint(x, -y);
    dc.LPtoDP(&sizeScroll);
    sizeScroll.cx = -sizeScroll.cx;
    pFieldView->OnScrollBy(sizeScroll, TRUE);
  }
}

void CFindWell::OnOK() 
{
  OnDblclkList1();
  GetFieldView()->GetDocument()->UpdateAllViews(0, IDD);
}