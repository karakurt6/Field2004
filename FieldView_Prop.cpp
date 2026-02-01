// FieldView_Prop.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "FieldView.h"
#include "FieldView_Prop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFieldView_Prop

IMPLEMENT_DYNAMIC(CFieldView_Prop, CPropertySheet)

CFieldView_Prop::CFieldView_Prop(CFieldView* pParentWnd): CPropertySheet("Field View", pParentWnd, 0)
{
  m_pageGrid.m_typeInit = pParentWnd->m_gridType;
  m_pageGrid.m_typePage = pParentWnd->m_gridType;
  m_pageGrid.m_pView = pParentWnd;
  AddPage(&m_pageGrid);

  m_pageData.m_typeInit = pParentWnd->m_dataType;
  m_pageData.m_typePage = pParentWnd->m_dataType;
  m_pageData.m_pView = pParentWnd;
  AddPage(&m_pageData);
}

CFieldView_Prop::~CFieldView_Prop()
{
}


BEGIN_MESSAGE_MAP(CFieldView_Prop, CPropertySheet)
	//{{AFX_MSG_MAP(CFieldView_Prop)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldView_Prop message handlers

/////////////////////////////////////////////////////////////////////////////
// CFieldView_PageGrid property page

IMPLEMENT_DYNCREATE(CFieldView_PageGrid, CPropertyPage)

CFieldView_PageGrid::CFieldView_PageGrid() : CPropertyPage(CFieldView_PageGrid::IDD)
{
	//{{AFX_DATA_INIT(CFieldView_PageGrid)
	m_typePage = -1;
	//}}AFX_DATA_INIT
}

CFieldView_PageGrid::~CFieldView_PageGrid()
{
}

void CFieldView_PageGrid::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFieldView_PageGrid)
	DDX_Radio(pDX, IDC_RADIO1, m_typePage);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFieldView_PageGrid, CPropertyPage)
	//{{AFX_MSG_MAP(CFieldView_PageGrid)
	ON_BN_CLICKED(IDC_RADIO1, OnPageChanged)
	ON_BN_CLICKED(IDC_RADIO2, OnPageChanged)
	ON_BN_CLICKED(IDC_RADIO3, OnPageChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldView_PageGrid message handlers

void CFieldView_PageGrid::OnOK() 
{
  if (m_pView->m_gridType != m_typePage)
  {
    m_pView->m_gridType = (CFieldView::GridType) m_typePage;

    CRect rectClient;
    m_pView->GetClientRect(&rectClient);
    m_pView->InvalidateRect(&rectClient);	  
  }
  CPropertyPage::OnOK();
}

void CFieldView_PageGrid::OnCancel() 
{
  if (m_pView->m_gridType != m_typeInit)
  {
    m_pView->m_gridType = (CFieldView::GridType) m_typeInit;

    CRect rectClient;
    m_pView->GetClientRect(&rectClient);
    m_pView->InvalidateRect(&rectClient);
  }
	CPropertyPage::OnCancel();
}

void CFieldView_PageGrid::OnPageChanged() 
{
  SetModified(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CFieldView_PageData property page

IMPLEMENT_DYNCREATE(CFieldView_PageData, CPropertyPage)

CFieldView_PageData::CFieldView_PageData() : CPropertyPage(CFieldView_PageData::IDD)
{
	//{{AFX_DATA_INIT(CFieldView_PageData)
	m_typePage = -1;
	//}}AFX_DATA_INIT
}

CFieldView_PageData::~CFieldView_PageData()
{
}

void CFieldView_PageData::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFieldView_PageData)
	DDX_Radio(pDX, IDC_RADIO1, m_typePage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFieldView_PageData, CPropertyPage)
	//{{AFX_MSG_MAP(CFieldView_PageData)
	ON_BN_CLICKED(IDC_RADIO1, OnPageChanged)
	ON_BN_CLICKED(IDC_RADIO2, OnPageChanged)
	ON_BN_CLICKED(IDC_RADIO3, OnPageChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldView_PageData message handlers

void CFieldView_PageData::OnPageChanged() 
{
  SetModified(TRUE);
}

void CFieldView_PageData::OnOK() 
{
  if (m_pView->m_dataType != m_typePage)
  {
    m_pView->m_dataType = (CFieldView::DataType) m_typePage;

    CRect rectClient;
    m_pView->GetClientRect(&rectClient);
    m_pView->InvalidateRect(&rectClient);	  
  }
  CPropertyPage::OnOK();
	CPropertyPage::OnOK();
}

void CFieldView_PageData::OnCancel() 
{
  if (m_pView->m_dataType != m_typeInit)
  {
    m_pView->m_dataType = (CFieldView::DataType) m_typeInit;

    CRect rectClient;
    m_pView->GetClientRect(&rectClient);
    m_pView->InvalidateRect(&rectClient);
  }
	CPropertyPage::OnCancel();
}
