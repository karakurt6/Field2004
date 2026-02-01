// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include <Inventor\Win\SoWin.h>

#include "Field.h"
#include "MainFrm.h"
#include "OverView.h"
#include "FieldView.h"
#include "FieldForm.h"
#include "FieldDoc.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_CROSS_SECTION, OnViewCrossSection)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CROSS_SECTION, OnUpdateViewCrossSection)
	ON_COMMAND(ID_PROBE_GRID, OnViewProbeGrid)
	ON_UPDATE_COMMAND_UI(ID_PROBE_GRID, OnUpdateViewProbeGrid)
	ON_COMMAND(ID_SATURATION, OnSaturation)
	ON_UPDATE_COMMAND_UI(ID_SATURATION, OnUpdateSaturation)
	ON_COMMAND(ID_RESDIALOG, OnResdialog)
	ON_UPDATE_COMMAND_UI(ID_RESDIALOG, OnUpdateResdialog)
	ON_COMMAND(ID_FIELD_EXPR, OnFieldExpr)
	ON_UPDATE_COMMAND_UI(ID_FIELD_EXPR, OnUpdateFieldExpr)
	ON_COMMAND(ID_FINDWELL, OnFindwell)
	ON_UPDATE_COMMAND_UI(ID_FINDWELL, OnUpdateFindwell)
	ON_COMMAND(ID_VIEW_WELLMATCH, OnViewWellmatch)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WELLMATCH, OnUpdateViewWellmatch)
//	ON_COMMAND(ID_FIELD_START3DVIEWER, OnFieldStart3dviewer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

  if (!m_wndCrossSection.Create(CCrossSection::IDD, this))
    return -1;

  if (!m_wndProbeDialog.Create(CProbeDialog::IDD, this))
    return -1;

  if (!m_wndCumulativeS.Create(CCumulativeS::IDD, this))
    return -1;

  if (!m_wndResDialog.Create(CResDialog::IDD, this))
    return -1;

  if (!m_wndIntegralProp.Create(CIntegralProp::IDD, this))
    return -1;

  if (!m_wndFindWell.Create(CFindWell::IDD, this))
    return -1;

  if (!m_wndLogtrackType.Create(CLogtrackType::IDD, this))
    return -1;

	SoWin::init(GetSafeHwnd());
	CWinApp* app = AfxGetApp();
	SoWin::setInstance(app->m_hInstance);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
  if (!m_wndSplitter1.CreateStatic(this, 2, 1))
  {
    TRACE0("Failed to CreateStatic() splitter window\n");
    return FALSE;
  }

  if (!m_wndSplitter2.CreateStatic(&m_wndSplitter1, 1, 2, \
    WS_CHILD|WS_VISIBLE|WS_BORDER, m_wndSplitter1.IdFromRowCol(0,0)))
  {
    TRACE0("Failed to create nested splitter\n");
    return FALSE;
  }

  if (!m_wndSplitter1.CreateView(1, 0, RUNTIME_CLASS(CFieldForm), CSize(100, 100), pContext))
  {
    TRACE0("Failed to create  form pane\n");
    return FALSE;
  }

  if (!m_wndSplitter2.CreateView(0, 1, pContext->m_pNewViewClass, CSize(100, 100), pContext))
  {
    TRACE0("Failed to create right pane in splitter window\n");
    return FALSE;
  }

  if (!m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(COverView), CSize(100, 100), pContext))
  {
    TRACE0("Failed to create overview pane\n");
    return FALSE;
  }

  m_pOverView = STATIC_DOWNCAST(COverView, m_wndSplitter2.GetPane(0, 0));
  m_pFieldView = STATIC_DOWNCAST(CFieldView, m_wndSplitter2.GetPane(0, 1));
  m_pFieldForm = STATIC_DOWNCAST(CFieldForm, m_wndSplitter1.GetPane(1, 0));

  return TRUE;
}

COverView* GetOverView()
{
  CMainFrame* pFrameWnd = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  return pFrameWnd->m_pOverView;
}

CFieldView* GetFieldView()
{
  CMainFrame* pFrameWnd = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  return pFrameWnd->m_pFieldView;
}

CFieldForm* GetFieldForm()
{
  CMainFrame* pFrameWnd = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  return pFrameWnd->m_pFieldForm;
}

CCrossSection* GetCrossSection()
{
  CMainFrame* pFrameWnd = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  return &pFrameWnd->m_wndCrossSection;
}

CProbeDialog* GetProbeDialog()
{
  CMainFrame* pFrameWnd = STATIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  return &pFrameWnd->m_wndProbeDialog;
}

CCumulativeS* GetCumulativeS()
{
  CMainFrame* pFrameWnd = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  if (!pFrameWnd)
    return 0;

  return &pFrameWnd->m_wndCumulativeS;
}

CResDialog* GetResDialog()
{
  CMainFrame* pFrameWnd = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  if (!pFrameWnd)
    return 0;

  return &pFrameWnd->m_wndResDialog;
}

CIntegralProp* GetIntegralProp()
{
  CMainFrame* pFrameWnd = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  if (!pFrameWnd)
    return 0;

  return &pFrameWnd->m_wndIntegralProp;
}

CFindWell* GetFindWell()
{
  CMainFrame* pFrameWnd = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  if (!pFrameWnd)
    return 0;

  return &pFrameWnd->m_wndFindWell;
}

CLogtrackType* GetLogtrackType()
{
  CMainFrame* pFrameWnd = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
  if (!pFrameWnd)
    return 0;

  return &pFrameWnd->m_wndLogtrackType;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);

  if (IsWindow(m_wndSplitter1) && IsWindow(m_wndSplitter2))
  {
    CFieldForm* pForm = m_pFieldForm;
    CActField* pDoc = pForm->GetDocument();

    int hh = 210;
    m_wndSplitter1.SetRowInfo(0, std::max(cy-hh,0), 0);
    m_wndSplitter1.SetRowInfo(1, hh, 0);

    int ww = int((double) std::max(hh, cy-hh-28) * pDoc->cx / pDoc->cy + 0.5);
    m_wndSplitter2.SetColumnInfo(0, ww, 0);
    m_wndSplitter2.SetColumnInfo(1, std::max(cx-ww,0), 0);

    m_wndSplitter2.RecalcLayout();
    m_wndSplitter1.RecalcLayout();
  }
}

void CMainFrame::OnViewCrossSection() 
{
  if (m_wndCrossSection.IsWindowVisible())
  {
    m_wndCrossSection.ShowWindow(SW_HIDE);
  }
  else
  {
    m_wndCrossSection.ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnUpdateViewCrossSection(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(m_wndCrossSection.IsWindowVisible());
}

void CMainFrame::OnViewProbeGrid() 
{
  if (m_wndProbeDialog.IsWindowVisible())
  {
    m_wndProbeDialog.ShowWindow(SW_HIDE);
  }
  else
  {
    // m_wndProbeDialog.UpdateData(FALSE);
    m_wndProbeDialog.ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnUpdateViewProbeGrid(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(m_wndProbeDialog.IsWindowVisible());
}

void CMainFrame::OnSaturation() 
{
  if (!m_wndCumulativeS.IsAvailable())
    return;

  if (m_wndCumulativeS.IsWindowVisible())
  {
    m_wndCumulativeS.ShowWindow(SW_HIDE);
  }
  else
  {
    m_wndCumulativeS.ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnUpdateSaturation(CCmdUI* pCmdUI) 
{
  if (!m_wndCumulativeS.IsAvailable())
  {
    pCmdUI->Enable(FALSE);
    return;
  }

  pCmdUI->SetCheck(m_wndCumulativeS.IsWindowVisible());
}

void CMainFrame::OnResdialog() 
{
  if (!m_wndResDialog.IsAvailable())
    return;

  if (m_wndResDialog.IsWindowVisible())
  {
    m_wndResDialog.ShowWindow(SW_HIDE);
  }
  else
  {
    m_wndResDialog.ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnUpdateResdialog(CCmdUI* pCmdUI) 
{
  if (!m_wndResDialog.IsAvailable())
  {
    pCmdUI->Enable(FALSE);
    return;
  }

  pCmdUI->SetCheck(m_wndResDialog.IsWindowVisible());
}

void CMainFrame::OnFieldExpr() 
{
  if (m_wndIntegralProp.IsWindowVisible())
  {
    m_wndIntegralProp.ShowWindow(SW_HIDE);
  }
  else
  {
    m_wndIntegralProp.ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnUpdateFieldExpr(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(m_wndIntegralProp.IsWindowVisible());
}

void CMainFrame::OnFindwell() 
{
  if (m_wndFindWell.IsWindowVisible())
  {
    m_wndFindWell.ShowWindow(SW_HIDE);
  }
  else
  {
    m_wndFindWell.ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnUpdateFindwell(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(m_wndFindWell.IsWindowVisible());
}

void CMainFrame::OnViewWellmatch()
{
  if (m_wndLogtrackType.IsWindowVisible())
  {
    m_wndLogtrackType.ShowWindow(SW_HIDE);
  }
  else
  {
    m_wndLogtrackType.ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnUpdateViewWellmatch(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_wndLogtrackType.IsWindowVisible());
}
