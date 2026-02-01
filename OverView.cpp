// OverView.cpp : implementation file
//

#include "stdafx.h"
#include "Field.h"
#include "FieldDoc.h"
#include "OverView.h"
#include "FieldView.h"
#include "FieldForm.h"
#include "CrossSection.h"
#include "CumulativeS.h"
#include "ResDialog.h"
#include "IntegralProp.h"
#include "FindWell.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COverView

IMPLEMENT_DYNCREATE(COverView, CView)

COverView::COverView()
{
  m_pColor = 0;
  m_bCapture = FALSE;
}

COverView::~COverView()
{
  delete[] m_pColor;
}


BEGIN_MESSAGE_MAP(COverView, CView)
	//{{AFX_MSG_MAP(COverView)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COverView drawing

static void CalculateActiveCells(CFieldView* pFieldView, double* bbox)
{
  CSize sizeTotal = pFieldView->GetTotalSize();

  int pg[4];
  pg[0] = 0;
  pg[1] = sizeTotal.cx;
  pg[2] = 0;
  pg[3] = sizeTotal.cy;

  CRect rectClient;
  pFieldView->GetClientRect(&rectClient);

  CClientDC dc(pFieldView);
  pFieldView->OnPrepareDC(&dc);
  dc.DPtoLP(&rectClient);

  int vp[4];
  vp[0] = rectClient.left;
  vp[1] = rectClient.right;
  vp[2] = rectClient.top;
  vp[3] = rectClient.bottom;

  CActField *pDoc = pFieldView->GetDocument();
  int cx = pDoc->cx, cy = pDoc->cy;
  double sx = (double) (pg[1] - pg[0]) / (cx-1);
  double sy = (double) (pg[3] - pg[2]) / (cy-1);

  bbox[0] = double(vp[0] - pg[0]) / sizeTotal.cx;
  bbox[1] = double(vp[1] - pg[0]) / sizeTotal.cx;
  bbox[2] = 1.0 - double(pg[3] + vp[2]) / sizeTotal.cy;
  bbox[3] = 1.0 - double(pg[3] + vp[3]) / sizeTotal.cy;
}

static void DrawPointField(CDC* pDC, int bbox[4])
{
  CFindWell *pFindWell = GetFindWell();
  CFieldForm* pForm = GetFieldForm();
  CActField* pDoc = pForm->GetDocument();
  if (!pFindWell->IsWindowVisible())
  {
    CFieldForm::ext_data_list *e_list = &pForm->m_listExt;
    for (CFieldForm::ext_data_list::iterator e_iter = e_list->begin(); e_iter != e_list->end(); ++e_iter)
    {
      int x = int(bbox[0] + (bbox[1] - bbox[0]) * (e_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = int(bbox[2] + (bbox[3] - bbox[2]) * (e_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      pDC->SetPixel(x, y, RGB(0,0,0));
      // pDC->Ellipse(x-1, y-1, x+1, y+1);
    }
    CFieldForm::inj_data_list *i_list = &pForm->m_listInj;
    for (CFieldForm::inj_data_list::iterator i_iter = i_list->begin(); i_iter != i_list->end(); ++i_iter)
    {
      int x = int(bbox[0] + (bbox[1] - bbox[0]) * (i_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = int(bbox[2] + (bbox[3] - bbox[2]) * (i_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      // pDC->SetPixel(x, y, RGB(255,255,255));
      pDC->SetPixel(x, y, RGB(0,0,0));
      // pDC->Ellipse(x-1, y-1, x+1, y+1);
    }
  }
  else
  {
    for (std::list<CFindWell::Well>::iterator it = pFindWell->m_listWell.begin(); \
      it != pFindWell->m_listWell.end(); ++it)
    {
      CFindWell::Well& well = *it;
      int x = int(bbox[0] + (bbox[1] - bbox[0]) * (well.x - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = int(bbox[2] + (bbox[3] - bbox[2]) * (well.y - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      pDC->SetPixel(x, y, RGB(0,0,0));
    }
  }
}

static void DrawCrossSection(CDC* pDC, int bbox[4])
{
  CCrossSection* pCS = GetCrossSection();
  if (!pCS->m_bVisible)
    return;

  CListCtrl* pp = &pCS->m_ctrlPoints;
  int n = pp->GetItemCount();
  if (n < 2)
    return;

  CFieldView* pFV = GetFieldView();
  CActField* pFD = pFV->GetDocument();
  CGdiObject *pPn = pDC->SelectStockObject(WHITE_PEN);
  coord_type* p = (coord_type*) pp->GetItemData(0);
  int x = int(bbox[0] + (bbox[1] - bbox[0]) * p->xcoord / (pFD->cx-1) + 0.5);
  int y = int(bbox[2] + (bbox[3] - bbox[2]) * (pFD->cy-1 - p->ycoord) / (pFD->cy-1) + 0.5);
  pDC->MoveTo(x, y);
  for (int i = 1; i < n; ++i)
  {
    p = (coord_type*) pp->GetItemData(i);
    x = int(bbox[0] + (bbox[1] - bbox[0]) * p->xcoord / (pFD->cx-1) + 0.5);
    y = int(bbox[2] + (bbox[3] - bbox[2]) * (pFD->cy-1 - p->ycoord) / (pFD->cy-1) + 0.5);
    pDC->LineTo(x, y);
  }
  if (pPn) pDC->SelectObject(pPn);
}

void COverView::OnDraw(CDC* pDC)
{
	CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  if (m_bitmap.GetSafeHandle())
  {
    CRect rectClient;
    GetClientRect(&rectClient);
    double cx = rectClient.Width(), cy = rectClient.Height();
    double dx = (pDoc->x2 - pDoc->x1), dy = (pDoc->y2 - pDoc->y1);
    bool ver = (cx * dy > cy * dx);
    double ss = (ver? (cy/dy): (cx/dx));
    double x0 = 0.5 * cx, sx = 0.5 * ss * dx;
    double y0 = 0.5 * cy, sy = 0.5 * ss * dy;
    int x1 = int(x0-sx+0.5), x2 = int(x0+sx+0.5);
    int y1 = int(y0-sy+0.5), y2 = int(y0+sy+0.5);

    CRect rectFill;
    CBrush brushWhite;
    VERIFY(brushWhite.CreateStockObject(WHITE_BRUSH));
    if (ver)
    {
      rectFill.SetRect(rectClient.top, rectClient.left, x1, rectClient.bottom);
    }
    else
    {
      rectFill.SetRect(rectClient.top, rectClient.left, rectClient.right, y1);
    }
    if (!rectFill.IsRectEmpty())
    {
      pDC->FillRect(&rectFill, &brushWhite);
    }

    CDC dcMem;
    dcMem.CreateCompatibleDC(pDC);
    CBitmap *pDefaultBM = dcMem.SelectObject(&m_bitmap);
    pDC->StretchBlt(x1, y1, x2-x1, y2-y1, &dcMem, 0, 0, pDoc->cx, pDoc->cy, SRCCOPY);
    if (pDefaultBM) dcMem.SelectObject(pDefaultBM);

    int bbox[4] = { x1, x2, y1, y2 };
    DrawCrossSection(pDC, bbox);
    DrawPointField(pDC, bbox);

    if (ver)
    {
      rectFill.SetRect(x2, y1, rectClient.right, rectClient.bottom);
    }
    else
    {
      rectFill.SetRect(x1, y2, rectClient.right, rectClient.bottom);
    }
    if (!rectFill.IsRectEmpty())
    {
      pDC->FillRect(&rectFill, &brushWhite);
    }

    double cells[4];
    CalculateActiveCells(GetFieldView(), cells);
    CRect rectCells(x1 + int((x2 - x1) * cells[0] + 0.5), y1 + int((y2 - y1) * cells[2] + 0.5), \
      x1 + int((x2 - x1) * cells[1] + 0.5), y1 + int((y2 - y1) * cells[3] + 0.5));
    pDC->Draw3dRect(&rectCells, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DDKSHADOW));
  }
}

/////////////////////////////////////////////////////////////////////////////
// COverView diagnostics

#ifdef _DEBUG
void COverView::AssertValid() const
{
	CView::AssertValid();
}

void COverView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CActField* COverView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CActField)));
	return (CActField*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COverView message handlers


static void inline ComputeColor(float z, int nz, const float* zz, \
  const color_type* clut, color_type* c_ptr)
{
  if (z <= zz[0])
  {
    c_ptr->r = clut[0].r;
    c_ptr->g = clut[0].g;
    c_ptr->b = clut[0].b;
  }
  else if (z >= zz[nz+1])
  {
    c_ptr->r = clut[nz+1].r;
    c_ptr->g = clut[nz+1].g;
    c_ptr->b = clut[nz+1].b;
  }
  else
  {
    int n = std::lower_bound(zz, zz+nz+2, z) - zz;
    double t = (z - zz[n-1]) / (zz[n] - zz[n-1]);
    c_ptr->r = clut[n-1].r + (clut[n].r - clut[n-1].r) * t;
    c_ptr->g = clut[n-1].g + (clut[n].g - clut[n-1].g) * t;
    c_ptr->b = clut[n-1].b + (clut[n].b - clut[n-1].b) * t;
  }
}

void COverView::OnInitialUpdate() 
{
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CClientDC dc(this);
  BITMAPINFOHEADER info;
  ZeroMemory(&info, sizeof(info));
  info.biSize = sizeof(info);
  info.biWidth = pDoc->cx;
  info.biHeight = pDoc->cy;
  info.biPlanes = 1;
  info.biBitCount = 24;
  info.biCompression = BI_RGB;

  if ((HBITMAP) m_bitmap != NULL)
    return;

  LPVOID pBits = 0;
  HBITMAP hBM = CreateDIBSection(dc, (LPBITMAPINFO) &info, DIB_RGB_COLORS, &pBits, NULL, 0);
  if (pBits)
  {
    typedef Color* color_ptr;
    int cx = pDoc->cx, cy = pDoc->cy;
    m_pColor = new color_ptr [cy];
    int stride = (cx * sizeof(Color) + 3) / 4 * 4;
    m_pColor[0] = (color_ptr) pBits;
    for (int j = 1; j < cy; ++j)
    {
      m_pColor[j] = (color_ptr)((char*) m_pColor[j-1]+stride);
    }

    const float* fn = pDoc->func;
    for (j = 0; j < cy; ++j)
    {
      for (int i = 0; i < cx; ++i)
      {
        color_type sh;
        ComputeColor(fn[j*cx+i], pDoc->clut.nz, pDoc->clut.zz, pDoc->clut.sh, &sh);
        m_pColor[j][i].r = (unsigned char) (255.0 * sh.r + 0.5);
        m_pColor[j][i].g = (unsigned char) (255.0 * sh.g + 0.5);
        m_pColor[j][i].b = (unsigned char) (255.0 * sh.b + 0.5);
      }
    }
    m_bitmap.Attach(hBM);
  }
	CView::OnInitialUpdate();
}

void COverView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CRect rectClient;
  GetClientRect(&rectClient);
  double cx = rectClient.Width(), cy = rectClient.Height();
  double dx = (pDoc->x2 - pDoc->x1), dy = (pDoc->y2 - pDoc->y1);
  double ss = (cx * dy > cy * dx? (cy/dy): (cx/dx));
  double x0 = 0.5 * cx, sx = 0.5 * ss * dx;
  double y0 = 0.5 * cy, sy = 0.5 * ss * dy;
  int x1 = int(x0-sx+0.5), x2 = int(x0+sx+0.5);
  int y1 = int(y0-sy+0.5), y2 = int(y0+sy+0.5);

  CRect rectTest(x1, y1, x2, y2);
  if (rectTest.PtInRect(point))
  {
    CFieldView *pFieldView = GetFieldView();
    CSize sizeTotal = pFieldView->GetTotalSize();
    int x = int(sizeTotal.cx * double(point.x - x1) / (x2 - x1) + 0.5);
    int y = int(sizeTotal.cy * double(point.y - y1) / (y2 - y1) + 0.5);

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

void COverView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CRect rectClient;
  GetClientRect(&rectClient);
  double cx = rectClient.Width(), cy = rectClient.Height();
  double dx = (pDoc->x2 - pDoc->x1), dy = (pDoc->y2 - pDoc->y1);
  double ss = (cx * dy > cy * dx? (cy/dy): (cx/dx));
  double x0 = 0.5 * cx, sx = 0.5 * ss * dx;
  double y0 = 0.5 * cy, sy = 0.5 * ss * dy;
  int x1 = int(x0-sx+0.5), x2 = int(x0+sx+0.5);
  int y1 = int(y0-sy+0.5), y2 = int(y0+sy+0.5);

  double cells[4];
  CalculateActiveCells(GetFieldView(), cells);
  CRect rectTest(x1 + int((x2 - x1) * cells[0] + 0.5), y1 + int((y2 - y1) * cells[2] + 0.5), \
    x1 + int((x2 - x1) * cells[1] + 0.5), y1 + int((y2 - y1) * cells[3] + 0.5));
  if (rectTest.PtInRect(point))
  {
    SetCapture();
    m_bCapture = TRUE;
    m_pointDrag = point;
  }
}

void COverView::OnLButtonUp(UINT nFlags, CPoint point) 
{
  if (m_bCapture)
  {
    ReleaseCapture();
    m_bCapture = FALSE;
  }
}

void COverView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CRect rectClient;
  GetClientRect(&rectClient);
  double cx = rectClient.Width(), cy = rectClient.Height();
  double dx = (pDoc->x2 - pDoc->x1), dy = (pDoc->y2 - pDoc->y1);
  double ss = (cx * dy > cy * dx? (cy/dy): (cx/dx));
  double x0 = 0.5 * cx, sx = 0.5 * ss * dx;
  double y0 = 0.5 * cy, sy = 0.5 * ss * dy;
  int x1 = int(x0-sx+0.5), x2 = int(x0+sx+0.5);
  int y1 = int(y0-sy+0.5), y2 = int(y0+sy+0.5);

  if (m_bCapture)
  {
    CFieldView *pFieldView = GetFieldView();
    CSize sizeTotal = pFieldView->GetTotalSize();
    int prev_x = int(sizeTotal.cx * double(m_pointDrag.x - x1) / (x2 - x1) + 0.5);
    int prev_y = int(sizeTotal.cy * double(m_pointDrag.y - y1) / (y2 - y1) + 0.5);
    int curr_x = int(sizeTotal.cx * double(point.x - x1) / (x2 - x1) + 0.5);
    int curr_y = int(sizeTotal.cy * double(point.y - y1) / (y2 - y1) + 0.5);
    CPoint prev(prev_x, -prev_y), curr(curr_x, -curr_y);
    m_pointDrag = point;

    CClientDC dc(pFieldView);
    pFieldView->OnPrepareDC(&dc);
    CSize sizeScroll = curr - prev;
    dc.LPtoDP(&sizeScroll);
    sizeScroll.cy = -sizeScroll.cy;
    pFieldView->OnScrollBy(sizeScroll, TRUE);
  }
}

BOOL COverView::OnEraseBkgnd(CDC* pDC) 
{
  return TRUE;
}

void COverView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  CActField* pDoc = GetDocument();
  CResDialog* pRes = GetResDialog();
  CCumulativeS* pSat = GetCumulativeS();
  CIntegralProp* pIntegralProp = GetIntegralProp();
  if (lHint == CIntegralProp::IDD && pIntegralProp->m_bActive)
  {
    float* fn = pIntegralProp->GetData();
    CActField::Palette* tt = pIntegralProp->GetPalette();
    int cx = pDoc->cx, cy = pDoc->cy;
    for (int j = 0; j < cy; ++j)
    {
      for (int i = 0; i < cx; ++i)
      {
        color_type sh;
        ComputeColor(fn[j*cx+i], tt->nz, tt->zz, tt->sh, &sh);
        m_pColor[j][i].r = (unsigned char) (255.0 * sh.r + 0.5);
        m_pColor[j][i].g = (unsigned char) (255.0 * sh.g + 0.5);
        m_pColor[j][i].b = (unsigned char) (255.0 * sh.b + 0.5);
      }
    }
  }
  else if ((lHint == CCumulativeS::IDD || pSat && IsWindow(pSat->m_hWnd) 
    && pSat->IsWindowVisible()) && pSat->init)
  {
    int cx = pDoc->cx, cy = pDoc->cy;
    for (int j = 0; j < cy; ++j)
    {
      for (int i = 0; i < cx; ++i)
      {
        int n = j*cx+i;
        float o = pSat->fn_o[n];
        float g = pSat->fn_g[n];
				float c = pSat->fn_c[n];
				color_type s;
				pSat->LookupColor(o, g, c, &s);
				m_pColor[j][i].r = (unsigned char) (255.0 * s.r + 0.5);
				m_pColor[j][i].g = (unsigned char) (255.0 * s.g + 0.5);
				m_pColor[j][i].b = (unsigned char) (255.0 * s.b + 0.5);
        // m_pColor[j][i].r = (unsigned char) (2.55 * o + 2.55 * g + 0.5);
        // m_pColor[j][i].g = (unsigned char) (1.28 * o + 2.55 * g + 0.5);
        // m_pColor[j][i].b = (unsigned char) (2.55 * (100 - o - g) + 0.5);
      }
    }
  }
  else if (lHint == CResDialog::IDD 
    && (!pRes->IsWindowVisible() || pSender == GetFieldForm()))
  {
    CResDialog::DataItem* pItem = 0;
    if (!pRes->m_listData.empty())
    {
      int n = pRes->m_nRowSel;
      for (CResDialog::DataList::iterator it = pRes->m_listData.begin(); --n; ++it);
      pItem = *it;
    }

    bool bOilSel = (pRes->m_nOilCol == pRes->m_nColSel);
    float *fn = (bOilSel? pItem->oil: pItem->gas) + 2;
    CActField::Palette* tt = (bOilSel? pRes->m_pOilTabl: pRes->m_pGasTabl);

    int cx = pDoc->cx, cy = pDoc->cy;
    for (int j = 0; j < cy; ++j)
    {
      for (int i = 0; i < cx; ++i)
      {
        color_type sh;
        ComputeColor(fn[j*cx+i], tt->nz, tt->zz, tt->sh, &sh);
        m_pColor[j][i].r = (unsigned char) (255.0 * sh.r + 0.5);
        m_pColor[j][i].g = (unsigned char) (255.0 * sh.g + 0.5);
        m_pColor[j][i].b = (unsigned char) (255.0 * sh.b + 0.5);
      }
    }
  }
  else if (m_pColor)
  {
    float *fn = pDoc->func;
    CActField::Palette* tt = &pDoc->clut;
    int cx = pDoc->cx, cy = pDoc->cy;
    for (int j = 0; j < cy; ++j)
    {
      for (int i = 0; i < cx; ++i)
      {
        color_type sh;
        ComputeColor(fn[j*cx+i], tt->nz, tt->zz, tt->sh, &sh);
        m_pColor[j][i].r = (unsigned char) (255.0 * sh.r + 0.5);
        m_pColor[j][i].g = (unsigned char) (255.0 * sh.g + 0.5);
        m_pColor[j][i].b = (unsigned char) (255.0 * sh.b + 0.5);
      }
    }
  }
  CView::OnUpdate(pSender, lHint, pHint);
}
