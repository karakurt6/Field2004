// FieldView.cpp : implementation of the CFieldView class
//

#include "stdafx.h"
#include "Field.h"

#include "FieldDoc.h"
#include "FieldView.h"
#include "FieldView_Prop.h"
#include "OverView.h"
#include "FieldForm.h"
#include "CrossSection.h"
#include "DataMapping.h"
#include "ProbeDialog.h"
#include "CumulativeS.h"
#include "ResDialog.h"
#include "IntegralProp.h"
#include "FindWell.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <valarray>
#include <list>
#include <numeric>
#include ".\fieldview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* Faster Line Segment Intersection   */
/* Franklin Antonio                   */

/* return values */
#define DONT_INTERSECT 0
#define DO_INTERSECT   1
#define COLLINEAR      2


/* The SAME_SIGNS macro assumes arithmetic where the exclusive-or    */
/* operation will work on sign bits.  This works for twos-complement,*/
/* and most other machine arithmetic.                                */
#define SAME_SIGNS( a, b ) \
  (((long) ((unsigned long) a ^ (unsigned long) b)) >= 0 )


/* The use of some short working variables allows this code to run   */
/* faster on 16-bit computers, but is not essential.  It should not  */
/* affect operation on 32-bit computers.  The short working variables*/
/* to not restrict the range of valid input values, as these were    */
/* constrained in any case, due to algorithm restrictions.           */

int lines_intersect(long x1, long y1, long x2, long y2, long x3, long y3, long x4, long y4, long* x, long* y) 
{

long Ax,Bx,Cx,Ay,By,Cy,d,e,f,num,offset;
short x1lo,x1hi,y1lo,y1hi;

Ax = x2-x1;
Bx = x3-x4;

if(Ax<0) {            /* X bound box test*/
  x1lo=(short)x2; x1hi=(short)x1;
  } else {
  x1hi=(short)x2; x1lo=(short)x1;
  }
if(Bx>0) {
  if(x1hi < (short)x4 || (short)x3 < x1lo) return DONT_INTERSECT;
  } else {
  if(x1hi < (short)x3 || (short)x4 < x1lo) return DONT_INTERSECT;
  }

Ay = y2-y1;
By = y3-y4;

if(Ay<0) {            /* Y bound box test*/
  y1lo=(short)y2; y1hi=(short)y1;
  } else {
  y1hi=(short)y2; y1lo=(short)y1;
  }
if(By>0) {
  if(y1hi < (short)y4 || (short)y3 < y1lo) return DONT_INTERSECT;
  } else {
  if(y1hi < (short)y3 || (short)y4 < y1lo) return DONT_INTERSECT;
  }


Cx = x1-x3;
Cy = y1-y3;
d = By*Cx - Bx*Cy;          /* alpha numerator*/
f = Ay*Bx - Ax*By;          /* both denominator*/
if(f>0) {           /* alpha tests*/
  if(d<0 || d>f) return DONT_INTERSECT;
  } else {
  if(d>0 || d<f) return DONT_INTERSECT;
  }

e = Ax*Cy - Ay*Cx;          /* beta numerator*/
if(f>0) {           /* beta tests*/
  if(e<0 || e>f) return DONT_INTERSECT;
  } else {
  if(e>0 || e<f) return DONT_INTERSECT;
  }

/*compute intersection coordinates*/

if(f==0) return COLLINEAR;
num = d*Ax;           /* numerator */
offset = SAME_SIGNS(num,f) ? f/2 : -f/2;    /* round direction*/
*x = x1 + (num+offset) / f;       /* intersection x */

num = d*Ay;
offset = SAME_SIGNS(num,f) ? f/2 : -f/2;
*y = y1 + (num+offset) / f;       /* intersection y */

return DO_INTERSECT;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define H_MARGIN 1000
#define V_MARGIN 1000
#define N_SHADE 6

typedef CActField::Color color_type;

void MeasureText(CDC* pDC, LPPOINT pCenter, const char* text, LPSIZE pSize)
{
  pDC->BeginPath();
  pDC->TextOut(pCenter->x, pCenter->y, text, lstrlen(text));
  pDC->EndPath();
  pDC->FlattenPath();

  int n = pDC->GetPath(0, 0, 0);
  std::valarray<POINT> points(n);
  std::valarray<BYTE> types(n);
  pDC->GetPath(&points[0], &types[0], n);

  int x1 = points[0].x, x2 = x1, y1 = points[0].y, y2 = y1;
  for (int i = 1; i < n; ++i)
  {
    int x = points[i].x, y = points[i].y;
    if (x < x1) x1 = x;
    if (x > x2) x2 = x;
    if (y > y2) y2 = y;
    if (y < y1) y1 = y;
  }

  pCenter->x = (x1+x2+1)/2;
  pCenter->y = (y1+y2+1)/2;
  pSize->cx = (x2-x1+1)/2;
  pSize->cy = (y2-y1+1)/2;
}

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

static inline COLORREF MakeColorRef(color_type* c_ptr)
{
  return RGB(int(255*c_ptr->r+0.5), int(255*c_ptr->g+0.5), int(255*c_ptr->b+0.5));
}

void DrawLabel(CDC* pDC, double x, double y, double p, \
  const char* text, double r, double dx, double dy, double x0, double y0)
{
  double a1 = 360.0 * atan2(dy, r+dx) / (2*M_PI);
  double a2 = 360.0 * atan2(dy+r, dx) / (2*M_PI);

  double d[9];
  d[0] = dy;
  d[1] = d[0] + M_PI * r / 2;
  d[2] = d[1] + 2 * dx;
  d[3] = d[2] + M_PI * r / 2;
  d[4] = d[3] + 2 * dy;
  d[5] = d[4] + M_PI * r / 2;
  d[6] = d[5] + 2 * dx;
  d[7] = d[6] + M_PI * r / 2;
  d[8] = d[7] + dy;

  if (p < 0)
  {
    p += ceil(-p);
  }
  if (p > 1)
  {
    p -= floor(p);
  }
  ASSERT(p >= 0.0 && p <= 1.0);

  p *= d[8];

  double bbox[8];
  if (p <= d[0])
  {
    double sy = p;

    x0 = 2 * x - x0 + dx + r;
    y0 = 2 * y - y0 + sy;
    
    bbox[0] = bbox[2] = x + r;
    bbox[1] = bbox[7] = y - dy + sy;
    bbox[3] = bbox[5] = y + dy + sy;
    bbox[4] = bbox[6] = x + 2 * dx + r;
  }

  else if (p <= d[1])
  {
    double a = (p - d[0]) / r;
    double sx = r * cos(a);
    double sy = r * sin(a);

    x0 = 2 * x - x0 + dx + sx;
    y0 = 2 * y - y0 + dy + sy;
    
    bbox[0] = bbox[2] = x + sx;
    bbox[1] = bbox[7] = y + sy;
    bbox[3] = bbox[5] = y + 2 * dy + sy;
    bbox[4] = bbox[6] = x + 2 * dx + sx;
  }

  else if (p <= d[2])
  {
    double sx = dx - (p - d[1]);

    x0 = 2 * x - x0 + sx;
    y0 = 2 * y - y0 + dy + r;

    bbox[0] = bbox[2] = x - dx + sx;
    bbox[1] = bbox[7] = y + r;
    bbox[3] = bbox[5] = y + 2 * dy + r;
    bbox[4] = bbox[6] = x + dx + sx;
  }  

  else if (p <= d[3])
  {
    double a = 0.5 * M_PI + (p - d[2]) / r;
    double sx = r * cos(a);
    double sy = r * sin(a);

    x0 = 2 * x - x0 - dx + sx;
    y0 = 2 * y - y0 + dy + sy;

    bbox[0] = bbox[2] = x - 2 * dx + sx;
    bbox[1] = bbox[7] = y + sy;
    bbox[3] = bbox[5] = y + 2 * dy + sy;
    bbox[4] = bbox[6] = x + sx;
  }

  else if (p <= d[4])
  {
    double sy = dy - (p - d[3]);

    x0 = 2 * x - x0 - dx - r;
    y0 = 2 * y - y0 + sy;

    bbox[0] = bbox[2] = x - 2 * dx - r;
    bbox[1] = bbox[7] = y - dy + sy;
    bbox[3] = bbox[5] = y + dy + sy;
    bbox[4] = bbox[6] = x - r;
  }

  else if (p <= d[5])
  {
    double a = M_PI + (p - d[4]) / r;
    double sx = r * cos(a);
    double sy = r * sin(a);

    x0 = 2 * x - x0 - dx + sx;
    y0 = 2 * y - y0 - dy + sy;

    bbox[0] = bbox[2] = x - 2 * dx + sx;
    bbox[1] = bbox[7] = y - 2 * dy + sy;
    bbox[3] = bbox[5] = y + sy;
    bbox[4] = bbox[6] = x + sx;
  }

  else if (p <= d[6])
  {
    double sx = -dx + (p - d[5]);

    x0 = 2 * x - x0 + sx;
    y0 = 2 * y - y0 - dy - r;

    bbox[0] = bbox[2] = x - dx + sx;
    bbox[1] = bbox[7] = y - 2 * dy - r;
    bbox[3] = bbox[5] = y - r;
    bbox[4] = bbox[6] = x + dx + sx;
  }  

  else if (p <= d[7])
  {
    double a = 1.5 * M_PI + (p - d[6]) / r;
    double sx = r * cos(a);
    double sy = r * sin(a);

    x0 = 2 * x - x0 + dx + sx;
    y0 = 2 * y - y0 - dy + sy;

    bbox[0] = bbox[2] = x + sx;
    bbox[1] = bbox[7] = y - 2 * dy + sy;
    bbox[3] = bbox[5] = y + sy;
    bbox[4] = bbox[6] = x + 2 * dx + sx;
  }

  else
  {
    double sy = (p-d[7])-dy;

    x0 = 2 * x - x0 + dx + r;
    y0 = 2 * y - y0 + sy;
    
    bbox[0] = bbox[2] = x + r;
    bbox[1] = bbox[7] = y - dy + sy;
    bbox[3] = bbox[5] = y + dy + sy;
    bbox[4] = bbox[6] = x + 2 * dx + r;
  }

  if (pDC)
  {
    pDC->TextOut(int(x0+0.5), int(y0+0.5), text);

    /*
    pDC->MoveTo(int(bbox[0]+0.5), int(bbox[1]+0.5));
    pDC->LineTo(int(bbox[2]+0.5), int(bbox[3]+0.5));
    pDC->LineTo(int(bbox[4]+0.5), int(bbox[5]+0.5));
    pDC->LineTo(int(bbox[6]+0.5), int(bbox[7]+0.5));
    pDC->LineTo(int(bbox[0]+0.5), int(bbox[1]+0.5));
    */

    // pDC->Ellipse(int(x-r+0.5), int(y-r+0.5), int(x+r+0.5), int(y+r+0.5));
  }
}

/////////////////////////////////////////////////////////////////////////////
// CColorPalette

CColorPalette::CColorPalette()
{
}

CColorPalette::~CColorPalette()
{
}

BOOL CColorPalette::CreateWnd(CFieldView* pParentWnd)
{
  if (!CreateEx(WS_EX_CLIENTEDGE|WS_EX_DLGMODALFRAME, AfxRegisterWndClass(CS_CLASSDC), NULL, \
    WS_CLIPSIBLINGS|WS_CHILD, 0, 0, 1, 1, pParentWnd->GetSafeHwnd(), NULL))
  {
    TRACE0("Failed to create CColorPalette window\n");
    ASSERT(FALSE);
    return FALSE;
  }

  return TRUE;
}


BEGIN_MESSAGE_MAP(CColorPalette, CWnd)
  //{{AFX_MSG_MAP(CColorPalette)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_CONTEXTMENU()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CColorPalette message handlers

BOOL CColorPalette::OnEraseBkgnd(CDC* pDC) 
{
  return TRUE;
}

void DrawColoring(CDC* pDC, const int* vp, const CActField::Palette* clut)
{
  CSize sizeText(1, 100 * pDC->GetDeviceCaps(LOGPIXELSY) / 720);
  pDC->DPtoLP(&sizeText);
  int nHeight = abs(sizeText.cy);

  pDC->SetBkMode(TRANSPARENT);
  UINT nDefaultTA = pDC->SetTextAlign(TA_BASELINE);

  CFont fontText;
  BOOL bOK = fontText.CreateFont(
    -nHeight, 0, // width and height
    0, 0, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline strike out
    DEFAULT_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip precision
    DEFAULT_QUALITY, // output quality
    DEFAULT_PITCH | FF_SWISS, // pitch and family
    "Arial"); // face name
  ASSERT(bOK);

  CFont* pDefaultFont = pDC->SelectObject(&fontText);

  CRect rc;
  rc.top = vp[2]+nHeight;
  rc.bottom = vp[3]-nHeight;
  rc.left = vp[0] + nHeight / 2;
  rc.right = rc.left + int(nHeight * GOLDEN_RATIO + 0.5);

  int n_label = rc.Height() / nHeight;
  int nz = clut->nz;
  float* zz = clut->zz;
  CActField::Color *sh = clut->sh;
  for (int i = 0; i <= n_label; ++i)
  {
    int y = rc.bottom + (rc.top - rc.bottom) * i / n_label;
    float z = zz[0] + (zz[nz+1] - zz[0]) * i / n_label;

    // comment out next line, if we use arithmetic scaling
    z = clut->bck(z);

    CString str;
    if (z < 10.0f)
      str.Format("%4.4f", z);
    else if (z < 100.0f)
      str.Format("%4.3f", z);
    else if (z < 1000.0f)
      str.Format("%4.2f", z);
    else if (z < 10000.0f)
      str.Format("%4.1f", z);
    else
      str.Format("%4.0f", z);

    CSize sz;
    CPoint pt(rc.right, y);
    MeasureText(pDC, &pt, str, &sz);
    DrawLabel(pDC, rc.right, y, 0.0, str, 0.5*nHeight, sz.cx, sz.cy, pt.x, pt.y);
  }
  
  CSize sizeLine(1, 1);
  pDC->DPtoLP(&sizeLine);
  int n_lines = rc.Height() / sizeLine.cy;
  for (i = 0; i <= n_lines; ++i)
  {
    color_type color;
    float z = zz[0] + (zz[nz+1] - zz[0]) * i / n_lines;

    ComputeColor(z, nz, zz, sh, &color);
    int y = rc.bottom + (rc.top - rc.bottom) * i / n_lines;
    CPen pen(PS_SOLID, 0, MakeColorRef(&color));
    CPen* pDefaultPn = pDC->SelectObject(&pen);
    pDC->MoveTo(rc.left, y);
    pDC->LineTo(rc.right, y);
    if (pDefaultPn) pDC->SelectObject(pDefaultPn);
  }

  CGdiObject* pDefaultBr = pDC->SelectStockObject(NULL_BRUSH);
  pDC->Rectangle(&rc);
  pDC->SelectObject(pDefaultBr);

  pDC->SetTextAlign(nDefaultTA);
  if (pDefaultFont) pDC->SelectObject(pDefaultFont);
}

void CColorPalette::OnPaint() 
{
  CPaintDC dc(this); // device context for painting

  CRect rc;
  GetClientRect(&rc);
  dc.FillSolidRect(&rc, GetSysColor(COLOR_BTNFACE));

  CFieldView* pView = STATIC_DOWNCAST(CFieldView, GetParent());
  CActField* pDoc = pView->GetDocument();

  int vp[4];
  vp[0] = rc.left;
  vp[1] = rc.right;
  vp[2] = rc.top;
  vp[3] = rc.bottom;

  CResDialog* pRes = GetResDialog();
  CIntegralProp* pIntegralProp = GetIntegralProp();
	if (pIntegralProp && pIntegralProp->GetPalette())
	{
		CActField::Palette *pal = pIntegralProp->GetPalette();
		DrawColoring(&dc, vp, pal);
	}
  else if (pRes && pRes->IsWindowVisible())
  {
    DrawColoring(&dc, vp, (pRes->m_nColSel == pRes->m_nOilCol? \
      pRes->m_pOilTabl: pRes->m_pGasTabl));
  }
  else if (pDoc->clut.zz)
  {
    DrawColoring(&dc, vp, &pDoc->clut);
  }
}

void CColorPalette::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  CActField::Palette* clut = 0;
  CResDialog* pRes = GetResDialog();
  CFieldView* pView = GetFieldView();
  CActField* pDoc = pView->GetDocument();
  if (pRes->IsWindowVisible())
  {
    clut = pRes->m_pOilTabl;
  }
  else
  {
    clut = &pDoc->clut;
  }

  CDataMapping dlg;
  int n = clut->nz+2;
  dlg.table.count = n;
  for (int i = 0; i < n; ++i)
  {
    float value = clut->bck(clut->zz[i]);

    COLORREF color = RGB(int(clut->sh[i].r * 255 + 0.5), \
      int(clut->sh[i].g * 255 + 0.5), int(clut->sh[i].b * 255 + 0.5));

    dlg.table.data[i].value = value;
    dlg.table.data[i].color = color;
  }

  if (dlg.DoModal() == IDOK)
  {
    delete[] clut->zz;
    delete[] clut->sh;
    int nn = dlg.table.count;
    clut->nz = nn-2;
    clut->zz = new float[nn];
    clut->sh = new CActField::Color[nn];
    for (int i = 0; i < nn; ++i)
    {
      clut->zz[i] = clut->fwd(dlg.table.data[i].value);
      clut->sh[i].r = GetRValue(dlg.table.data[i].color) / 255.0;
      clut->sh[i].g = GetGValue(dlg.table.data[i].color) / 255.0;
      clut->sh[i].b = GetBValue(dlg.table.data[i].color) / 255.0;
    }
    Invalidate();
    pDoc->UpdateAllViews(GetFieldForm());
  }
}

/////////////////////////////////////////////////////////////////////////////
// CFieldView

IMPLEMENT_DYNCREATE(CFieldView, CScrollView)

BEGIN_MESSAGE_MAP(CFieldView, CScrollView)
  //{{AFX_MSG_MAP(CFieldView)
  ON_WM_ERASEBKGND()
  ON_WM_CONTEXTMENU()
  ON_COMMAND(ID_FIELD_VIEW_PROP, OnFieldViewProp)
  ON_COMMAND(ID_FIELD_SCALE, OnFieldScale)
  ON_COMMAND(ID_FIELD_COLORING, OnFieldColoring)
  ON_WM_LBUTTONDBLCLK()
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONDOWN()
  ON_COMMAND(ID_FIELD_QUERYWELLBORE, OnFieldQuerywellbore)
//  ON_COMMAND(ID_FIELD_START3DVIEWER, OnFieldStart3dviewer)
  //}}AFX_MSG_MAP
  // Standard printing commands
  ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
  ON_COMMAND(ID_FIELD_START3DVIEWER, OnFieldStart3dviewer)
	ON_COMMAND(ID_INCLINOMETRY, OnInclinometry)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldView construction/destruction

UINT CFieldView::s_nDragPointFormat;

CFieldView::CFieldView()
{
  s_nDragPointFormat = RegisterClipboardFormat(_T("DragPoint"));
  ASSERT(s_nDragPointFormat);

  m_gridType = GRID_CELLS;
  m_dataType = DATA_NODES;
  m_bScale = 1;
  m_bColoring = 1;
  m_enumHit = HIT_NONE;

  SetScrollSizes(MM_HIMETRIC, CSize(100, 100));
}

CFieldView::~CFieldView()
{
}

BOOL CFieldView::PreCreateWindow(CREATESTRUCT& cs)
{
  // TODO: Modify the Window class or styles here by modifying
  //  the CREATESTRUCT cs
  cs.style |= WS_CLIPCHILDREN;

  return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFieldView drawing

void DrawDataLevels(CDC* pDC, const int* pg, const int* vp, int cx, int cy, const float* fn, \
  int nz, const float* zz, const color_type* sh)
{
  int n_level = N_SHADE * nz;
  std::valarray<float> level(n_level);
  std::valarray<color_type> shade(n_level+1);
  float dz = zz[nz+1] - zz[0];
  for (int i = 0; i < n_level; ++i)
  {
    level[i] = zz[0] + (i+1) * dz / (n_level+1);
    ComputeColor(level[i], nz, zz, sh, &shade[i]);
  }
  ComputeColor(zz[0] + n_level * dz / (n_level+1), nz, zz, sh, &shade[n_level]);

  CSize sizePel(1, 1);
  pDC->DPtoLP(&sizePel);
  double sx = (double) (pg[1] - pg[0]) / (cx-1);
  double sy = (double) (pg[3] - pg[2]) / (cy-1);

  int x1 = (int) floor((vp[0] - pg[0]) / sx), \
    x2 = (int) ceil((vp[1] - pg[0]) / sx), \
    y2 = (int) ceil((pg[3] + vp[2]) / sy), \
    y1 = (int) floor((pg[3] + vp[3]) / sy);

  if (x1 < 0) x1 = 0;
  if (x2 >= cx) x2 = cx-1;
  if (y1 < 0) y1 = 0;
  if (y2 >= cy) y2 = cy-1;
  // int ss[6] = { x1, x2, cx, y1, y2, cy };

  // vertex ver;
  // std::valarray<path> con(n_level-1), pgn(n_level-1);
  // conrec(ss, fn, n_level-2, &level[0], ver, &con[0], &pgn[0]);

  vertex_type ver;
  std::valarray<edge_type> con(n_level), pgn(n_level+2);
  marching_squares(cx, cy, x1, x2, y1, y2, fn, n_level, &level[0], ver, &con[0], &pgn[0]);

  std::list<POINT> pp;
  std::list<int> nn;
  for (i = 0; i <= n_level; ++i)
  {
    if (pgn[i].empty())
    {
      nn.push_back(0);
      continue;
    }

    std::list<int> acc;
    for (edge_type::const_iterator p_iter = pgn[i].begin(); p_iter != pgn[i].end(); ++p_iter)
    {
      curve_type::const_iterator c_iter = p_iter->begin();

      POINT prev;
      const coord_type& v = ver[*c_iter];
      prev.x = pg[0] + int(sx * v.xcoord + 0.5);
      prev.y = -pg[2] - int(sy * (cy-1 - v.ycoord) + 0.5);
      pp.push_back(prev);

      int n = 1;
      while (++c_iter != p_iter->end())
      {
        POINT curr;
        const coord_type& v = ver[*c_iter];
        curr.x = pg[0] + int(sx * v.xcoord + 0.5);
        curr.y = -pg[2] - int(sy * (cy-1 - v.ycoord) + 0.5);
        if (prev.x != curr.x || prev.y != curr.y)
        {
          pp.push_back(curr);
          prev = curr;
          ++n;
        }
      }
      acc.push_back(n);
    }
    nn.push_back(acc.size());
    nn.splice(nn.end(), acc);
  }

  std::valarray<POINT> p_arr(pp.size());
  std::copy(pp.begin(), pp.end(), &p_arr[0]);

  std::valarray<int> n_arr(nn.size());
  std::copy(nn.begin(), nn.end(), &n_arr[0]);

  pDC->SetPolyFillMode(WINDING);
  pDC->SetBkMode(TRANSPARENT);

  CGdiObject *pDefaultPen = pDC->SelectStockObject(NULL_PEN);
  POINT* p_ptr = &p_arr[0];
  int* n_ptr = &n_arr[0];
  for (i = 0; i <= n_level; ++i)
  {
    int n = *n_ptr++;
    if (!n) continue;
    CBrush brushLevel(MakeColorRef(&shade[i]));
    CBrush *pDefaultBrush = pDC->SelectObject(&brushLevel);
    pDC->PolyPolygon(p_ptr, n_ptr, n);
    if (pDefaultBrush) pDC->SelectObject(pDefaultBrush);
    p_ptr += std::accumulate(n_ptr, n_ptr + n, 0);
    n_ptr += n;
  }
  if (pDefaultPen) pDC->SelectObject(pDefaultPen);
}

void DrawDataCells(CDC* pDC, const int* pg, const int* vp, int cx, int cy, const float* fn, \
  int nz, const float* zz, const color_type* sh)
{
  CSize sizePel(1, 1);
  pDC->DPtoLP(&sizePel);
  double sx = (double) (pg[1] - pg[0]) / (cx-1);
  double sy = (double) (pg[3] - pg[2]) / (cy-1);

  int x1 = (int) floor((vp[0] - pg[0]) / sx), \
    x2 = (int) ceil((vp[1] - pg[0]) / sx), \
    y2 = (int) ceil((pg[3] + vp[2]) / sy), \
    y1 = (int) floor((pg[3] + vp[3]) / sy);

  if (x1 < 0) x1 = 0;
  if (x2 >= cx) x2 = cx-1;
  if (y1 < 0) y1 = 0;
  if (y2 >= cy) y2 = cy-1;

  CRect rectCell;
  CGdiObject *pDefaultPen = pDC->SelectStockObject(NULL_PEN);
  for (int j = y1; j <= y2; ++j)
  {
    double y = sy * (cy-1 - j);
    rectCell.top = -pg[2] - (int) (y + 0.5 * (j > 0? (sy + 2 * sizePel.cy): 0.0) + 0.5);
    rectCell.bottom = -pg[2] -(int) (y - 0.5 * (j < cy-1? (sy + 2 * sizePel.cy): 0.0) + 0.5);
    for (int i = x1; i <= x2; ++i)
    {
      double x = sx * i;
      rectCell.left = pg[0] + (int) (x - 0.5 * (i > 0? (sx + 2 * sizePel.cx): 0.0) + 0.5);
      rectCell.right = pg[0] + (int) (x + 0.5 * (i < cx-1? (sx + 2 * sizePel.cx): 0.0) + 0.5);

      color_type color;
      ComputeColor(fn[j*cx+i], nz, zz, sh, &color);
      CBrush brushCell(MakeColorRef(&color));
      CBrush *pDefaultBrush = pDC->SelectObject(&brushCell);
      pDC->Rectangle(&rectCell);
      pDC->SelectObject(pDefaultBrush);
    }
  }
  if (pDefaultPen) pDC->SelectObject(pDefaultPen);
}

void DrawGridNodes(CDC* pDC, const int* pg, const int* vp, int cx, int cy)
{
  CSize sizePel(1, 1);
  pDC->DPtoLP(&sizePel);
  double sx = (double) (pg[1] - pg[0]) / (cx-1);
  double sy = (double) (pg[3] - pg[2]) / (cy-1);

  int x1 = (int) floor((vp[0] - pg[0]) / sx + 0.5), \
    x2 = (int) ceil((vp[1] - pg[0]) / sx + 0.5), \
    y2 = (int) ceil((pg[3] + vp[2]) / sy + 0.5), \
    y1 = (int) floor((pg[3] + vp[3]) / sy + 0.5);

  if (x1 < 0) x1 = 0;
  if (x2 >= cx) x2 = cx-1;
  if (y1 < 0) y1 = 0;
  if (y2 >= cy) y2 = cy-1;

  CFont fontVer;
  BOOL bOK = fontVer.CreateFont(
    -MulDiv(200, pDC->GetDeviceCaps(LOGPIXELSY), 72), 0, // height and width
    2700, 2700, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline, strike out
    DEFAULT_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip default precision
    DEFAULT_QUALITY, // output quality,
    DEFAULT_PITCH | FF_SWISS, // pitch and family
    "Arial"); // face name
  ASSERT(bOK);

  pDC->SetBkMode(TRANSPARENT);
  CFont *pDefaultFt = pDC->SelectObject(&fontVer);
  for (int j = y1; j < y2; ++j)
  {
    int y = -pg[2] - int(sy * (cy-1 - j - 0.5) + 0.5);
    if (vp[2]-300 > y)
    {
      pDC->MoveTo(vp[0]+300, y);
      pDC->LineTo(vp[1], y);

      CString str;
      str.Format("%d", j+1);
      CSize sizeText = pDC->GetTextExtent(str);
      pDC->TextOut(vp[0], y-(sizeText.cx+1)/2, str);
    }
  }

  CFont fontHor;
  bOK = fontHor.CreateFont(
    -MulDiv(200, pDC->GetDeviceCaps(LOGPIXELSY), 72), 0, // height and width
    0, 0, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline, strike out
    DEFAULT_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip default precision
    DEFAULT_QUALITY, // output quality,
    DEFAULT_PITCH | FF_SWISS, // pitch and family
    "Arial"); // face name
  ASSERT(bOK);

  pDC->SelectObject(&fontHor);
  for (int i = x1; i < x2; ++i)
  {
    int x = pg[0] + int(sx * (i + 0.5) - sizePel.cx);
    if (vp[0]+300 < x)
    {
      pDC->MoveTo(x, vp[2]-300);
      pDC->LineTo(x, vp[3]);

      CString str;
      str.Format("%d", i+1);
      CSize sizeText = pDC->GetTextExtent(str);
      pDC->TextOut(x-(sizeText.cx+1)/2, vp[2], str);
    }
  }
  if (pDefaultFt) pDC->SelectObject(pDefaultFt);
}

void DrawDataNodes(CDC* pDC, const int* pg, const int* vp, int cx, int cy, const float* fn, \
  int nz, const float* zz, const color_type* sh)
{
  CSize sizePel(1, 1);
  pDC->DPtoLP(&sizePel);
  double sx = (double) (pg[1] - pg[0]) / (cx-1);
  double sy = (double) (pg[3] - pg[2]) / (cy-1);

  int x1 = (int) floor((vp[0] - pg[0]) / sx), \
    x2 = (int) ceil((vp[1] - pg[0]) / sx), \
    y2 = (int) ceil((pg[3] + vp[2]) / sy), \
    y1 = (int) floor((pg[3] + vp[3]) / sy);

  if (x1 < 0) x1 = 0;
  if (x2 >= cx) x2 = cx-1;
  if (y1 < 0) y1 = 0;
  if (y2 >= cy) y2 = cy-1;

  CRect rectCell;
  CGdiObject *pDefaultPen = pDC->SelectStockObject(NULL_PEN);
  for (int j = y1; j < y2; ++j)
  {
    double y = sy * (cy-1 - j);
    rectCell.top = -pg[2] - (int) (y + 0.5); 
    rectCell.bottom = -pg[2] - (int) (y - sy + 0.5);
    for (int i = x1; i < x2; ++i)
    {
      double x = sx * i;
      rectCell.left = pg[0] + (int) floor(x + 0.5);
      rectCell.right = pg[0] + (int) ceil(x + sx + 0.5);

      CRect rectSample;
      int n = j*cx+i;
      color_type color;
      const int nx = 6, ny = 6;
      float z11 = fn[n], z21 = fn[n+1], z12 = fn[n+cx], z22 = fn[n+cx+1];
      for (int jj = 0; jj < ny; ++jj)
      {
        rectSample.top = rectCell.top + jj * (rectCell.bottom - rectCell.top) / ny - sizePel.cy;
        rectSample.bottom = rectCell.top + (jj+1) * (rectCell.bottom - rectCell.top) / ny + sizePel.cy;
        float v = (jj+0.5f) / ny;
        for (int ii = 0; ii < nx; ++ii)
        {
          rectSample.left = rectCell.left + ii * (rectCell.right - rectCell.left) / nx - sizePel.cx;
          rectSample.right = rectCell.left + (ii+1) * (rectCell.right - rectCell.left) / nx + sizePel.cx;
          float u = (ii+0.5f) / ny, z = z11*(1-u)*(1-v)+z21*u*(1-v)+z12*(1-u)*v+z22*u*v;
          ComputeColor(z, nz, zz, sh, &color);
          CBrush brushCell(MakeColorRef(&color));
          CBrush *pDefaultBrush = pDC->SelectObject(&brushCell);
          pDC->Rectangle(&rectSample);
          pDC->SelectObject(pDefaultBrush);
        }
      }

    }
  }
  if (pDefaultPen) pDC->SelectObject(pDefaultPen);
}

void DrawGridCells(CDC* pDC, const int* pg, const int* vp, int cx, int cy)
{
  CSize sizePel(1, 1);
  pDC->DPtoLP(&sizePel);
  double sx = (double) (pg[1] - pg[0]) / (cx-1);
  double sy = (double) (pg[3] - pg[2]) / (cy-1);

  int vv[4];
  vv[0] = vp[0] + 300;
  vv[1] = vp[1];
  vv[2] = vp[2] - 300;
  vv[3] = vp[3];

  int x1 = (int) floor((vv[0] - pg[0]) / sx + 0.5), \
    x2 = (int) ceil((vv[1] - pg[0]) / sx + 0.5), \
    y2 = (int) ceil((pg[3] + vv[2]) / sy + 0.5), \
    y1 = (int) floor((pg[3] + vv[3]) / sy + 0.5);

  if (x1 < 0) x1 = 0;
  if (x2 >= cx) x2 = cx-1;
  if (y1 < 0) y1 = 0;
  if (y2 >= cy) y2 = cy-1;

  CFont fontVer;
  BOOL bOK = fontVer.CreateFont(
    -MulDiv(200, pDC->GetDeviceCaps(LOGPIXELSY), 72), 0, // height and width
    2700, 2700, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline, strike out
    DEFAULT_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip default precision
    DEFAULT_QUALITY, // output quality,
    DEFAULT_PITCH | FF_SWISS, // pitch and family
    "Arial"); // face name
  ASSERT(bOK);

  pDC->SetBkMode(TRANSPARENT);
  CFont *pDefaultFt = pDC->SelectObject(&fontVer);
  for (int j = y1; j <= y2; ++j)
  {
    int y = -pg[2] - int(sy * (cy-1 - j) + 0.5);
    if (vv[2] > y)
    {
      pDC->MoveTo(vv[0], y);
      pDC->LineTo(vv[1], y);

      CString str;
      str.Format("%d", j);
      CSize sizeText = pDC->GetTextExtent(str);
      pDC->TextOut(vp[0], y-(sizeText.cx+1)/2, str);
    }
  }

  CFont fontHor;
  bOK = fontHor.CreateFont(
    -MulDiv(200, pDC->GetDeviceCaps(LOGPIXELSY), 72), 0, // height and width
    0, 0, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline, strike out
    DEFAULT_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip default precision
    DEFAULT_QUALITY, // output quality,
    DEFAULT_PITCH | FF_SWISS, // pitch and family
    "Arial"); // face name
  ASSERT(bOK);

  pDC->SelectObject(&fontHor);
  for (int i = x1; i <= x2; ++i)
  {
    int x = pg[0] + int(sx * i + 0.5);
    if (vv[0] < x)
    {
      pDC->MoveTo(x, vv[2]);
      pDC->LineTo(x, vv[3]);

      CString str;
      str.Format("%d", i);
      CSize sizeText = pDC->GetTextExtent(str);
      pDC->TextOut(x-(sizeText.cx+1)/2, vp[2], str);
    }
  }
  if (pDefaultFt) pDC->SelectObject(pDefaultFt);
}

void DrawGridStub(CDC*, const int*, const int*, int, int)
{
}

static void (*DrawData[3])(CDC*, const int*, const int*, \
  int, int, const float*, int, const float*, const color_type*) =
{
  DrawDataCells, DrawDataNodes, DrawDataLevels
};

static void (*DrawGrid[3])(CDC*, const int*, const int*, int, int) =
{
  DrawGridStub, DrawGridCells, DrawGridNodes
};

void DrawScale(CDC* pDC, int x0, int y0)
{
  int dx = 1000, dy = 100;
  CGdiObject *pDefaultBr = pDC->SelectStockObject(BLACK_BRUSH);
  for (int i = 0; i < 5; ++i)
  {
    int x1 = x0 + dx * i / 5, x2 = x1 + dx / 5;
    if (i & 1)
    {
      pDC->Rectangle(x1, y0-dy, x2, y0);
    }
    else
    {
      pDC->Rectangle(x1, y0, x2, y0+dy);
    }
  }

  for (i = 1; i < 4; ++i)
  {
    int x1 = x0 + dx * i, x2 = x1 + dx;
    if (i & 1)
    {
      pDC->Rectangle(x1, y0-dy, x2, y0);
    }
    else
    {
      pDC->Rectangle(x1, y0, x2, y0+dy);
    }
  }

  pDC->SelectStockObject(HOLLOW_BRUSH);
  pDC->Rectangle(x0, y0-dy, x0+4*dx, y0+dy);
  // pDC->Draw3dRect(x0, y0-dy, 4*dx, dy, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DDKSHADOW));
  if (pDefaultBr) pDC->SelectObject(pDefaultBr);

  CFont fontHor;
  BOOL bOK = fontHor.CreateFont(
    -MulDiv(160, pDC->GetDeviceCaps(LOGPIXELSY), 72), 0, // height and width
    0, 0, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline, strike out
    DEFAULT_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip default precision
    DEFAULT_QUALITY, // output quality,
    DEFAULT_PITCH | FF_SWISS, // pitch and family
    "Arial"); // face name
  ASSERT(bOK);

  pDC->SetBkMode(TRANSPARENT);
  CFont *pDefaultFt = pDC->SelectObject(&fontHor);
  for (i = 0; i <= 4; ++i)
  {
    CString str;
    str.Format("%d", -150 + 150 * i);
    CSize sizeText = pDC->GetTextExtent(str);
    pDC->TextOut(x0-(sizeText.cx+1)/2+dx*i, y0-2*dy, str);
  }
  if (pDefaultFt) pDC->SelectObject(pDefaultFt);
}

struct point_feature
{ 
  const char* text;
  int x0, y0;
  int dr, dx, dy;
  int sx, sy;
  double param;
  bool inj;
	COLORREF cr;
};

void DrawPointField(CDC* pDC, const int* pg, const int* vp)
{
  CFieldForm* pForm = GetFieldForm();
  CActField* pDoc = pForm->GetDocument();

  double dx = (pg[1] - pg[0]);
  double dy = (pg[3] - pg[2]);

  double x1 = (pDoc->cx-1) * (vp[0]+300) / dx;
  double x2 = (pDoc->cx-1) * vp[1] / dx;
  double y1 = (1-pDoc->cy) * (vp[2]-300) / dy;
  double y2 = (1-pDoc->cy) * vp[3] / dy;

  pDC->SetBkMode(TRANSPARENT);
  UINT nDefaultTA = pDC->SetTextAlign(TA_BASELINE);

  CSize sizeText(1, 100 * pDC->GetDeviceCaps(LOGPIXELSY) / 720);
  pDC->DPtoLP(&sizeText);
  int nHeight = abs(sizeText.cy);
  int radius = (nHeight+1)/2;

  CFont fontText;
  BOOL bOK = fontText.CreateFont(
    -nHeight, 0, // width and height
    0, 0, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline strike out
    DEFAULT_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip precision
    DEFAULT_QUALITY, // output quality
    DEFAULT_PITCH | FF_ROMAN, // pitch and family
    "Times New Roman"); // face name
  ASSERT(bOK);

  CFont fontLabel;
  bOK = fontLabel.CreateFont(
    nHeight, 0, // width and height
    0, 0, // escapement and orientation
    FW_DONTCARE, // weight
    FALSE, FALSE, FALSE, // italic, underline strike out
    SYMBOL_CHARSET, // character set
    OUT_DEFAULT_PRECIS, // output precision
    CLIP_DEFAULT_PRECIS, // clip precision
    DEFAULT_QUALITY, // output quality
    DEFAULT_PITCH | FF_DONTCARE, // pitch and family
    "GSI Oil and Gas"); // face name
  ASSERT(bOK);

  CFont* pDefaultFont = pDC->SelectObject(&fontText);

  std::list<point_feature> problem;
  CFindWell* pFindWell = GetFindWell();
  if (!pFindWell->IsWindowVisible())
  {
    CFieldForm::inj_data_list* i_list = &pForm->m_listInj;
    for (CFieldForm::inj_data_list::iterator i_iter = i_list->begin(); i_iter != i_list->end(); ++i_iter)
    {
      int x = int(pg[0] + (pg[1] - pg[0]) * (i_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = -int(pg[2] + (pg[3] - pg[2]) * (i_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
      {
        CPoint pointCenter(x, y);
        MeasureText(pDC, &pointCenter, i_iter->well, &sizeText);
        point_feature data = 
        { 
          i_iter->well, x, y, radius,
          pointCenter.x, pointCenter.y, 
          sizeText.cx, sizeText.cy, 0.0, 
					true, RGB(0,0,0)
        };
        problem.push_back(data);
      }
    }

    CFieldForm::ext_data_list* e_list = &pForm->m_listExt;
    for (CFieldForm::ext_data_list::iterator e_iter = e_list->begin(); e_iter != e_list->end(); ++e_iter)
    {
      int x = int(pg[0] + (pg[1] - pg[0]) * (e_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = -int(pg[2] + (pg[3] - pg[2]) * (e_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
      {
        CPoint pointCenter(x, y);
        MeasureText(pDC, &pointCenter, e_iter->well, &sizeText);
        point_feature data = 
        { 
          e_iter->well, x, y, radius,
          pointCenter.x, pointCenter.y, 
          sizeText.cx, sizeText.cy, 0.0, 
					false, RGB(0,0,0)
        };
        problem.push_back(data);
      }
    }
  }
  else
  {
    for (std::list<CFindWell::Well>::iterator it = pFindWell->m_listWell.begin(); \
      it != pFindWell->m_listWell.end(); ++it)
    {
      CFindWell::Well& well = *it;
      int x = int(pg[0] + (pg[1] - pg[0]) * (well.x - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = -int(pg[2] + (pg[3] - pg[2]) * (well.y - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
      {
        CPoint pointCenter(x, y);
        MeasureText(pDC, &pointCenter, well.name, &sizeText);
        point_feature data = 
        { 
          well.name, x, y, radius,
          pointCenter.x, pointCenter.y, 
          sizeText.cx, sizeText.cy, 0.0, 
					true, RGB(0,0,0)
        };
				int nIndex = pFindWell->m_wndList.GetCurSel();
				if (nIndex != LB_ERR)
				{
					CFindWell::Well* w = (CFindWell::Well*) pFindWell->m_wndList.GetItemData(nIndex);
					if (w == &(*it))
					{
						data.cr = RGB(255, 255, 255);
					}
				}
        problem.push_back(data);
      }
    }
  }

  for (std::list<point_feature>::iterator p_iter = problem.begin(); p_iter != problem.end(); ++p_iter)
  {
    point_feature& p1 = *p_iter;
    int sx = p1.dr + 2 * p1.sx, sy = p1.dr + 2 * p1.sy;
    CRect R1(p1.x0-sx, p1.y0-sy, p1.x0+sx, p1.y0+sy);

    double sum_x = 0.0, sum_y = 0.0;
    for (std::list<point_feature>::iterator c_iter = problem.begin(); c_iter != problem.end(); ++c_iter)
    {
      point_feature& p2 = *c_iter;
      if (&p1 == &p2)
        continue;

      // check for a potential conflict
      sx = p2.dr + 2 * p2.sx, sy = p2.dr + 2 * p2.sy;
      CRect R, R2(p2.x0-sx, p2.y0-sy, p2.x0+sx, p2.y0+sy);
      if (R.IntersectRect(&R1, &R2))
      {
        double dx = p2.x0 - p1.x0, dy = p2.y0 - p1.y0, ss = p2.dr / hypot(dx, dy);
        sum_x += dx * ss;
        sum_y += dy * ss;
      }
    }

    if (sum_x != 0.0 || sum_y != 0.0)
    {
      double rad = atan2(sum_y, sum_x) + M_PI;
      double ang = 360.0 * rad / (2*M_PI);

      const int n = 9;
      static double threshold[n] = 
      {
        30, 60, 120, 150, 210, 240, 300, 330, 360
      };
      static double param[n] =
      {
        0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 0.0
      };
      for (int i = 0; i < n; ++i)
      {
        if (ang < threshold[i])
        {
          p1.param = param[i];
          break;
        }
      }
    }
  }

  for (p_iter = problem.begin(); p_iter != problem.end(); ++p_iter)
  {
    const point_feature& pf = *p_iter;

    pDC->SetTextAlign(TA_CENTER);
    pDC->SetTextColor(pf.cr);
    pDC->SelectObject(&fontLabel);
    pDC->TextOut(pf.x0, pf.y0+pf.dr, (pf.inj? "j": "p"), 1);

    pDC->SetTextAlign(TA_BASELINE);
    pDC->SetTextColor(0);
    pDC->SelectObject(&fontText);
    DrawLabel(pDC, pf.x0, pf.y0, pf.param, pf.text, pf.dr, pf.sx, pf.sy, pf.dx, pf.dy);
    // pDC->Ellipse(p.x0-p.dr, p.y0-p.dr, p.x0+p.dr, p.y0+p.dr);
  }

  pDC->SetTextAlign(nDefaultTA);
  if (pDefaultFont) pDC->SelectObject(pDefaultFont);
}

static void DrawCrossSection(CDC* pDC, const int* pg, const int* vp)
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
  int x = int(pg[0] + (pg[1] - pg[0]) * p->xcoord / (pFD->cx-1) + 0.5);
  int y = -int(pg[2] + (pg[3] - pg[2]) * (pFD->cy-1 - p->ycoord) / (pFD->cy-1) + 0.5);
  pDC->MoveTo(x, y);
  for (int i = 1; i < n; ++i)
  {
    p = (coord_type*) pp->GetItemData(i);
    x = int(pg[0] + (pg[1] - pg[0]) * p->xcoord / (pFD->cx-1) + 0.5);
    y = -int(pg[2] + (pg[3] - pg[2]) * (pFD->cy-1 - p->ycoord) / (pFD->cy-1) + 0.5);
    pDC->LineTo(x, y);
  }
  if (pPn) pDC->SelectObject(pPn);
}

static void DrawPolygon(CDC* pDC, const int* pg, const int* vp)
{
  CDigitizeDlg* pDlg = &GetIntegralProp()->m_wndDigitize;
  if (!pDlg->IsWindowVisible() || pDlg->data.empty())
    return;

  CFieldView* pView = GetFieldView();
  CActField* pDoc = pView->GetDocument();
  CGdiObject *pPen = pDC->SelectStockObject(WHITE_PEN);
  std::list<coord_type>::iterator p = pDlg->data.begin();
  int x = int(pg[0] + (pg[1] - pg[0]) * p->xcoord / (pDoc->cx - 1) + 0.5);
  int y = -int(pg[2] + (pg[3] - pg[2]) * (pDoc->cy - 1 - p->ycoord) / (pDoc->cy - 1) + 0.5);
  pDC->MoveTo(x, y);
  while (++p != pDlg->data.end())
  {
    x = int(pg[0] + (pg[1] - pg[0]) * p->xcoord / (pDoc->cx - 1) + 0.5);
    y = -int(pg[2] + (pg[3] - pg[2]) * (pDoc->cy - 1 - p->ycoord) / (pDoc->cy - 1) + 0.5);
    pDC->LineTo(x, y);
  }

  {
    coord_type& p = pDlg->data.front();
    x = int(pg[0] + (pg[1] - pg[0]) * p.xcoord / (pDoc->cx - 1) + 0.5);
    y = -int(pg[2] + (pg[3] - pg[2]) * (pDoc->cy - 1 - p.ycoord) / (pDoc->cy - 1) + 0.5);
    pDC->LineTo(x, y);
  }
  if (pPen) pDC->SelectObject(pPen);
}

void CFieldView::OnDraw(CDC* pDC)
{
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

	if (!pDoc->func)
		return;

  int pg[4];
  if (!pDC->IsPrinting())
  {
    CSize sizeTotal = GetTotalSize();
    pg[0] = 0;
    pg[1] = sizeTotal.cx;
    pg[2] = 0;
    pg[3] = sizeTotal.cy;
  }
  else
  {
    CSize sizeRes;
    sizeRes.cx = pDC->GetDeviceCaps(HORZRES);
    sizeRes.cy = pDC->GetDeviceCaps(VERTRES);

    pDC->DPtoLP(&sizeRes);
    sizeRes.cx -= 2 * H_MARGIN;
    sizeRes.cy -= 2 * V_MARGIN;

    double cx = sizeRes.cx, cy = sizeRes.cy;
    double dx = (pDoc->x2 - pDoc->x1), dy = (pDoc->y2 - pDoc->y1);
    double ss = (cx * dy > cy * dx? (cy/dy): (cx/dx));
    double x0 = 0.5 * cx, sx = 0.5 * ss * dx;
    double y0 = 0.5 * cy, sy = 0.5 * ss * dy;

    pg[0] = H_MARGIN + int(x0 - sx + 0.5);
    pg[1] = H_MARGIN + int(x0 + sx + 0.5);
    pg[2] = V_MARGIN + int(y0 - sy + 0.5);
    pg[3] = V_MARGIN + int(y0 + sy + 0.5);

    // TRACE("Horizontal scale: 1 cm = %f m\n", dx * 1.0e3 / (pg[1] - pg[0]));
    // TRACE("Vertical scale: 1 cm = %f m\n", dy * 1.0e3 / (pg[3] - pg[2]));
  }

  int vp[4];
  if (!pDC->IsPrinting())
  {
    CDC dcMem;
    VERIFY(dcMem.CreateCompatibleDC(pDC));
    OnPrepareDC(&dcMem);
  
    CRect rectClient;
    GetClientRect(&rectClient);

    CBitmap bitmapBuf;
    VERIFY(bitmapBuf.CreateCompatibleBitmap(pDC, rectClient.Width(), rectClient.Height()));
    CBitmap* pDefaultBm = dcMem.SelectObject(&bitmapBuf);

    pDC->DPtoLP(&rectClient);
    // pDC->GetClipBox(&rectClient);
    vp[0] = rectClient.left;
    vp[1] = rectClient.right;
    vp[2] = rectClient.top;
    vp[3] = rectClient.bottom;

    int cx = pDoc->cx, cy = pDoc->cy;
    CCumulativeS* pSat = GetCumulativeS();
    CResDialog* pRes = GetResDialog();
    CResDialog::DataItem* pItem = 0;
    if (pRes && !pRes->m_listData.empty())
    {
      int n = pRes->m_nRowSel;
      for (CResDialog::DataList::iterator it = pRes->m_listData.begin(); --n; ++it);
      pItem = *it;
    }
    
    CIntegralProp* pIntegralProp = GetIntegralProp();
    if (pIntegralProp && pIntegralProp->GetData())
    {
      float *fn = pIntegralProp->GetData();
      CActField::Palette* pal = pIntegralProp->GetPalette();
      DrawData[m_dataType](&dcMem, pg, vp, cx, cy, fn, pal->nz, pal->zz, pal->sh);
    }
    else if (pSat && pSat->IsWindowVisible())
    {
      pSat->DrawDataNodes(&dcMem, pg, vp);
    }
    else if (pRes && pRes->IsWindowVisible() && pItem != 0)
    {
      bool bOilSel = (pRes->m_nColSel == pRes->m_nOilCol);
      float* func = (bOilSel? pItem->oil: pItem->gas) + 2;
      CActField::Palette* clut = (bOilSel? pRes->m_pOilTabl: pRes->m_pGasTabl);
      DrawData[m_dataType](&dcMem, pg, vp, cx, cy, func, \
        clut->nz, clut->zz, clut->sh);
    }
    else if (pDoc->func)
    {
      DrawData[m_dataType](&dcMem, pg, vp, cx, cy, pDoc->func, \
        pDoc->clut.nz, pDoc->clut.zz, pDoc->clut.sh);
    }

    DrawGrid[m_gridType](&dcMem, pg, vp, cx, cy);

    DrawPolygon(&dcMem, pg, vp);
    DrawCrossSection(&dcMem, pg, vp);
    DrawPointField(&dcMem, pg, vp);


    if (m_bScale)
      DrawScale(&dcMem, vp[0]+1000, vp[3]+1000);

    /*
    if (m_bColoring)
      DrawColoring(&dcMem, vp, pDoc->n_level, pDoc->level, pDoc->color);
    */

    pDC->BitBlt(rectClient.left, rectClient.top, \
      rectClient.Width(), rectClient.Height(), &dcMem, rectClient.left, rectClient.top, SRCCOPY);

    if (pDefaultBm) dcMem.SelectObject(pDefaultBm);
  }
  else
  {
    CRect rectClip;
    pDC->GetClipBox(&rectClip);

    vp[0] = rectClip.left;
    vp[1] = rectClip.right;
    vp[2] = rectClip.top;
    vp[3] = rectClip.bottom;

    int cx = pDoc->cx, cy = pDoc->cy;
    DrawData[m_dataType](pDC, pg, vp, cx, cy, pDoc->func, \
      pDoc->clut.nz, pDoc->clut.zz, pDoc->clut.sh);

    DrawGrid[m_gridType](pDC, pg, vp, pDoc->cx, pDoc->cy);
  }
}

void CFieldView::OnInitialUpdate()
{
  CScrollView::OnInitialUpdate();
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  double x1 = pDoc->x1, x2 = pDoc->x2;
  double y1 = pDoc->y1, y2 = pDoc->y2;

  // calculate scroll sizes based on scale 1 cm = 150 m
  CSize sizeTot;
  sizeTot.cx = int((x2 - x1) * 1.0e3 / 150 + 0.5);
  sizeTot.cy = int((y2 - y1) * 1.0e3 / 150 + 0.5);
  SetScrollSizes(MM_HIMETRIC, sizeTot);

  TRACE("Horizontal scale: 1 cm = %f m\n", (x2 - x1) * 1.0e3 / sizeTot.cx);
  TRACE("Vertical scale: 1 cm = %f m\n", (y2 - y1) * 1.0e3 / sizeTot.cy);

  if (m_wndColorPalette.IsWindowVisible())
    m_wndColorPalette.Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CFieldView diagnostics

#ifdef _DEBUG
void CFieldView::AssertValid() const
{
  CScrollView::AssertValid();
}

void CFieldView::Dump(CDumpContext& dc) const
{
  CScrollView::Dump(dc);
}

CActField* CFieldView::GetDocument() // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CActField)));
  return (CActField*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFieldView message handlers

BOOL CFieldView::OnEraseBkgnd(CDC* pDC) 
{
  // return CScrollView::OnEraseBkgnd(pDC);
  return TRUE;
}

void CFieldView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  CCumulativeS* pSat = GetCumulativeS();
  if (pSat->IsWindowVisible())
    return;

  // CMenu menu;
  // VERIFY(menu.LoadMenu(IDR_FIELD_VIEW));
  // CMenu *pMenu = menu.GetSubMenu(0);
  // ASSERT(pMenu != NULL);

  CMenu menu;
  VERIFY(menu.CreatePopupMenu());
  menu.AppendMenu(MF_STRING | (m_bColoring? MF_CHECKED: MF_UNCHECKED), \
    ID_FIELD_COLORING, "Color &Mapping");
  menu.AppendMenu(MF_STRING | (m_bScale? MF_CHECKED: MF_UNCHECKED), \
    ID_FIELD_SCALE, "Scale &Bar");

  {
    CPoint pp = point;
    ScreenToClient(&pp);
    CClientDC dc(this);
    OnPrepareDC(&dc);
    dc.DPtoLP(&pp);

    CSize sizeTot = GetTotalSize();
    CActField* pDoc = GetDocument();
    CFieldDoc::Point p;
    p.x = (double) (pDoc->cx-1) * pp.x / (sizeTot.cx-1);
    p.y = (double) (pDoc->cy-1) * (sizeTot.cy + pp.y) / (sizeTot.cy-1);

    long bore;
    char name[16];
    if (pDoc->RangeQuery(p, 0.1, 1, &bore) && LookupWellName(pDoc, bore, name))
    {
      m_nBoreSelected = bore;
      menu.AppendMenu(MF_STRING | (pDoc->Info(bore, 0, 0) == 0? MF_GRAYED: 0), ID_FIELD_QUERYWELLBORE, TEXT("Geophysics"));
      menu.AppendMenu(MF_STRING, ID_FIELD_START3DVIEWER, TEXT("3D Fragment"));
			menu.AppendMenu(MF_STRING, ID_INCLINOMETRY, TEXT("Inclinometry"));
    }
  }

  menu.AppendMenu(MF_SEPARATOR);
  menu.AppendMenu(MF_STRING, ID_FIELD_VIEW_PROP, "Properties");
  menu.SetDefaultItem(ID_FIELD_VIEW_PROP);
  menu.TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd(), NULL);
}

void CFieldView::OnFieldViewProp() 
{
  CFieldView_Prop dlg(this);
  dlg.DoModal();
}

void CFieldView::DoOverview()
{
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CRect rectClient;
  CClientDC dc(this);
  OnPrepareDC(&dc);
  GetClientRect(&rectClient);
  dc.DPtoLP(&rectClient);

  int cell[4];
  CSize sizeTotal = GetTotalSize();
  cell[0] = (int) floor(double(pDoc->cx-1) * rectClient.left / sizeTotal.cx);
  cell[1] = (int) ceil(double(pDoc->cx-1) * rectClient.right / sizeTotal.cx);
  cell[2] = (int) ceil(double(pDoc->cy-1) * (sizeTotal.cy + rectClient.bottom) / sizeTotal.cy);
  cell[3] = (int) floor(double(pDoc->cy-1) * (sizeTotal.cy + rectClient.top) / sizeTotal.cy);

  COverView* pOverView = GetOverView();
  pOverView->GetClientRect(&rectClient);
  double cx = rectClient.Width(), cy = rectClient.Height();
  double dx = (pDoc->x2 - pDoc->x1), dy = (pDoc->y2 - pDoc->y1);
  bool ver = (cx * dy > cy * dx);
  double ss = (ver? (cy/dy): (cx/dx));
  double x0 = 0.5 * cx, sx = 0.5 * ss * dx;
  double y0 = 0.5 * cy, sy = 0.5 * ss * dy;
  int x1 = int(x0-sx+0.5), x2 = int(x0+sx+0.5);
  int y1 = int(y0-sy+0.5), y2 = int(y0+sy+0.5);

  rectClient.SetRect(x1 + (x2 - x1) * cell[0] / (pDoc->cx-1), 
    y1 + (y2 - y1) * (pDoc->cy-1 - cell[3]) / (pDoc->cy-1),
    x1 + (x2 - x1) * cell[1] / (pDoc->cx-1),
    y1 + (y2 - y1) * (pDoc->cy-1 - cell[2]) / (pDoc->cy-1));
  rectClient.InflateRect(2, 2);

  pOverView->InvalidateRect(&rectClient);
}

BOOL CFieldView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll) 
{
  int xOrig, x;
  int yOrig, y;

  // don't scroll if there is no valid scroll range (ie. no scroll bar)
  CScrollBar* pBar;
  DWORD dwStyle = GetStyle();
  pBar = GetScrollBarCtrl(SB_VERT);
  if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
    (pBar == NULL && !(dwStyle & WS_VSCROLL)))
  {
    // vertical scroll bar not enabled
    sizeScroll.cy = 0;
  }
  pBar = GetScrollBarCtrl(SB_HORZ);
  if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
    (pBar == NULL && !(dwStyle & WS_HSCROLL)))
  {
    // horizontal scroll bar not enabled
    sizeScroll.cx = 0;
  }

  // adjust current x position
  xOrig = x = GetScrollPos(SB_HORZ);
  int xMax = GetScrollLimit(SB_HORZ);
  x += sizeScroll.cx;
  if (x < 0)
    x = 0;
  else if (x > xMax)
    x = xMax;

  // adjust current y position
  yOrig = y = GetScrollPos(SB_VERT);
  int yMax = GetScrollLimit(SB_VERT);
  y += sizeScroll.cy;
  if (y < 0)
    y = 0;
  else if (y > yMax)
    y = yMax;

  // did anything change?
  if (x == xOrig && y == yOrig)
    return FALSE;

  if (bDoScroll)
  {
    // do scroll and update scroll positions
    // ScrollWindow(-(x-xOrig), -(y-yOrig));
    DoOverview();

    if (x != xOrig)
      SetScrollPos(SB_HORZ, x);
    if (y != yOrig)
      SetScrollPos(SB_VERT, y);

    DoOverview();
    Invalidate();
  }
  return TRUE;
}

void CFieldView::OnFieldScale()
{
  m_bScale ^= 1;
  Invalidate();
}

void CFieldView::OnFieldColoring() 
{
  m_bColoring ^= 1;

  CRect rc;
  GetClientRect(&rc);
  m_wndColorPalette.ShowWindow(m_bColoring && rc.Width() > 100 && rc.Height() > 80? SW_SHOW: SW_HIDE);
}

void CFieldView::SyncWellList(CPoint point)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  CRect rectClient;
  GetClientRect(&rectClient);
  dc.DPtoLP(&rectClient);
  dc.DPtoLP(&point);

  CSize sizeText(1, 100 * dc.GetDeviceCaps(LOGPIXELSY) / 720);
  dc.DPtoLP(&sizeText);
  int nHeight = abs(sizeText.cy);
  int rr = (nHeight+1)/2;

  int pg[4];
  CSize sizeTot = GetTotalSize();
  pg[0] = 0;
  pg[1] = sizeTot.cx;
  pg[2] = 0;
  pg[3] = sizeTot.cy;

  int vp[4];
  vp[0] = rectClient.left;
  vp[1] = rectClient.right;
  vp[2] = rectClient.top;
  vp[3] = rectClient.bottom;

  CFieldForm* pForm = GetFieldForm();
  CActField* pDoc = GetDocument();
  CFieldForm::inj_data_list* i_list = &pForm->m_listInj;
  for (CFieldForm::inj_data_list::iterator i_iter = i_list->begin(); i_iter != i_list->end(); ++i_iter)
  {
    int x = int(pg[0] + (pg[1] - pg[0]) * (i_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
    int y = -int(pg[2] + (pg[3] - pg[2]) * (i_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
    if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
    {
      int dx = x - point.x, dy = y - point.y;
      if (dx*dx + dy*dy < rr * rr)
      {
        LVFINDINFO info;
        info.flags = LVFI_STRING;
        info.psz = i_iter->well;
        int n = pForm->m_ctrlInj.FindItem(&info);
        if (n != -1)
        {
          pForm->m_ctrlInj.EnsureVisible(n, FALSE);
          pForm->m_ctrlInj.SetItemState(n, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        }
        return;
      }
    }
  }

  CFieldForm::ext_data_list* e_list = &pForm->m_listExt;
  for (CFieldForm::ext_data_list::iterator e_iter = e_list->begin(); e_iter != e_list->end(); ++e_iter)
  {
    int x = int(pg[0] + (pg[1] - pg[0]) * (e_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
    int y = -int(pg[2] + (pg[3] - pg[2]) * (e_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
    if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
    {
      int dx = x - point.x, dy = y - point.y;
      if (dx*dx + dy*dy < rr * rr)
      {
        LVFINDINFO info;
        info.flags = LVFI_STRING;
        info.psz = e_iter->well;
        int n = pForm->m_ctrlExt.FindItem(&info);
        if (n != -1)
        {
          pForm->m_ctrlExt.EnsureVisible(n, FALSE);
          pForm->m_ctrlExt.SetItemState(n, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        }
        return;
      }
    }
  }
}

void CFieldView::AppendCtrlPoint(CPoint point)
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&point);

  CSize sizeTot = GetTotalSize();
  CActField* pDoc = GetDocument();
  coord_type* p = new coord_type;
  p->xcoord = (double) (pDoc->cx-1) * point.x / sizeTot.cx;
  p->ycoord = (double) (pDoc->cy-1) * (sizeTot.cy + point.y) / sizeTot.cy;
  AppendCtrlPoint(p);
}

void CFieldView::UpdateCtrlPoint(int n)
{
  CListCtrl* pp = &GetCrossSection()->m_ctrlPoints;
  coord_type* p = (coord_type*) pp->GetItemData(n);
  CClientDC dc(this);
  OnPrepareDC(&dc);
  CRect rectClient;
  GetClientRect(&rectClient);
  dc.DPtoLP(&rectClient);

  CSize sizeText(1, 100 * dc.GetDeviceCaps(LOGPIXELSY) / 720);
  dc.DPtoLP(&sizeText);
  int nHeight = abs(sizeText.cy);
  int rr = (nHeight+1)/2;
  int pg[4];

  CSize sizeTot = GetTotalSize();
  pg[0] = 0;
  pg[1] = sizeTot.cx;
  pg[2] = 0;
  pg[3] = sizeTot.cy;

  int vp[4];
  vp[0] = rectClient.left;
  vp[1] = rectClient.right;
  vp[2] = rectClient.top;
  vp[3] = rectClient.bottom;

  CPoint point;
  CActField* pDoc = GetDocument();
  point.x = int(sizeTot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
  point.y = int(sizeTot.cy * p->ycoord / (pDoc->cy-1) - sizeTot.cy + 0.5);
  const char* well = 0;

  CFieldForm* pForm = GetFieldForm();
  CFieldForm::inj_data_list* i_list = &pForm->m_listInj;
  for (CFieldForm::inj_data_list::iterator i_iter = i_list->begin(); i_iter != i_list->end(); ++i_iter)
  {
    int x = int(pg[0] + (pg[1] - pg[0]) * (i_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
    int y = -int(pg[2] + (pg[3] - pg[2]) * (i_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
    if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
    {
      int dx = x - point.x, dy = y - point.y;
      if (dx*dx + dy*dy < rr * rr)
      {
        well = i_iter->well;
        break;
      }
    }
  }
  if (!well)
  {
    CFieldForm::ext_data_list* e_list = &pForm->m_listExt;
    for (CFieldForm::ext_data_list::iterator e_iter = e_list->begin(); e_iter != e_list->end(); ++e_iter)
    {
      int x = int(pg[0] + (pg[1] - pg[0]) * (e_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = -int(pg[2] + (pg[3] - pg[2]) * (e_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
      {
        int dx = x - point.x, dy = y - point.y;
        if (dx*dx + dy*dy < rr * rr)
        {
          well = e_iter->well;
        }
      }
    }
  }

  CString str;
  pp->SetItemText(n, 0, well);
  str.Format("%lf", pDoc->x1 + (pDoc->x2 - pDoc->x1) * p->xcoord / (pDoc->cx-1));
  pp->SetItemText(n, 1, str);
  str.Format("%f", pDoc->y1 + (pDoc->y2 - pDoc->y1) * p->ycoord / (pDoc->cy-1));
  pp->SetItemText(n, 2, str);
  pp->SetItemData(n, (DWORD) p);
}

void CFieldView::AppendCtrlPoint(coord_type* p)
{
  CCrossSection *pCS = GetCrossSection();
  if (!pCS->m_bVisible)
    return;

  CListCtrl *pp = &pCS->m_ctrlPoints;
  int n = pp->GetItemCount();
  pp->InsertItem(n, "");
  pp->SetItemData(n, (DWORD) p);
  UpdateCtrlPoint(n);
  pp->EnsureVisible(n, FALSE);
  GetDocument()->UpdateAllViews(0);
}

void CFieldView::UpdateCrossSection()
{
  CClientDC dc(this);
  OnPrepareDC(&dc);
  CRect rectClient;
  GetClientRect(&rectClient);
  dc.DPtoLP(&rectClient);

  CSize sizeText(1, 100 * dc.GetDeviceCaps(LOGPIXELSY) / 720);
  dc.DPtoLP(&sizeText);
  int nHeight = abs(sizeText.cy);
  int rr = (nHeight+1)/2;

  int pg[4];
  CSize sizeTot = GetTotalSize();
  pg[0] = 0;
  pg[1] = sizeTot.cx;
  pg[2] = 0;
  pg[3] = sizeTot.cy;

  int vp[4];
  vp[0] = rectClient.left;
  vp[1] = rectClient.right;
  vp[2] = rectClient.top;
  vp[3] = rectClient.bottom;

  CCrossSection *pCS = GetCrossSection();
  CFieldForm* pForm = GetFieldForm();
  CActField* pDoc = GetDocument();
  CListCtrl *pp = &pCS->m_ctrlPoints;
  int n = pp->GetItemCount();
  for (int i = 0; i < n; ++i)
  {
    coord_type* p = (coord_type*) pp->GetItemData(i);
    int x0 = int(p->xcoord * sizeTot.cx / (pDoc->cx-1) + 0.5);
    int y0 = int(p->ycoord * sizeTot.cy / (pDoc->cy-1) - sizeTot.cy + 0.5);
    const char* well = 0;

    CFieldForm::inj_data_list* i_list = &pForm->m_listInj;
    for (CFieldForm::inj_data_list::iterator i_iter = i_list->begin(); i_iter != i_list->end(); ++i_iter)
    {
      int x = int(pg[0] + (pg[1] - pg[0]) * (i_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
      int y = -int(pg[2] + (pg[3] - pg[2]) * (i_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
      if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
      {
        int dx = x - x0, dy = y - y0;
        if (dx*dx + dy*dy < rr * rr)
        {
          well = i_iter->well;
          break;
        }
      }
    }
    if (!well)
    {
      CFieldForm::ext_data_list* e_list = &pForm->m_listExt;
      for (CFieldForm::ext_data_list::iterator e_iter = e_list->begin(); e_iter != e_list->end(); ++e_iter)
      {
        int x = int(pg[0] + (pg[1] - pg[0]) * (e_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
        int y = -int(pg[2] + (pg[3] - pg[2]) * (e_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);
        if (vp[0]+300 <= x && x <= vp[1] && vp[3] <= y && y <= vp[2]-300)
        {
          int dx = x - x0, dy = y - y0;
          if (dx*dx + dy*dy < rr * rr)
          {
            well = e_iter->well;
            break;
          }
        }
      }

      pp->SetItemText(i, 0, well);
    }
  }
}

void CFieldView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  if (GetProbeDialog()->IsWindowVisible())
  {
    CClientDC dc(this);
    OnPrepareDC(&dc);
    dc.DPtoLP(&point);

    CSize sizeTot = GetTotalSize();
    CActField* pDoc = GetDocument();

    int x_index = int((double) (pDoc->cx-1) * point.x / sizeTot.cx + 0.5);
    int y_index = int((double) (pDoc->cy-1) * (sizeTot.cy + point.y) / sizeTot.cy + 0.5);
    GetProbeDialog()->FillGrid(x_index, y_index);
  }
  if (GetCrossSection()->IsWindowVisible())
  {
    AppendCtrlPoint(point);
  }
  SyncWellList(point);
}

int CFieldView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CScrollView::OnCreate(lpCreateStruct) == -1)
    return -1;

  if (!m_wndColorPalette.CreateWnd(this))
    return -1;

  if (m_bColoring)
    m_wndColorPalette.ShowWindow(SW_SHOW);
  
  m_cmdDropTarget.Register(this);

  return 0;
}

void CFieldView::OnSize(UINT nType, int cx, int cy) 
{
  CScrollView::OnSize(nType, cx, cy);

  if (cx > 120 && cy > 80)
  {
    m_wndColorPalette.MoveWindow(cx-110, 20, 90, cy-40);
    if (m_bColoring && !m_wndColorPalette.IsWindowVisible())
      m_wndColorPalette.ShowWindow(SW_SHOW);
  }
  else if (m_wndColorPalette.IsWindowVisible())
  {
    m_wndColorPalette.ShowWindow(SW_HIDE);
  }
}


void CFieldView::HitCrossSection(CPoint pointTest)
{
  static const int tol = 50;
  CCrossSection *pCS = GetCrossSection();
  if (!pCS->m_bVisible)
  {
    m_enumHit = HIT_NONE;
    return;
  }

  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&pointTest);
  CSize sizeTotal = GetTotalSize();
  const CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CListCtrl* pp = &pCS->m_ctrlPoints;
  int n = pp->GetItemCount();
  for (int i = 0; i < n; ++i)
  {
    const coord_type* p = (coord_type*) pp->GetItemData(i);
    int x = int(sizeTotal.cx * p->xcoord / (pDoc->cx-1) + 0.5);
    int y = -int(sizeTotal.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
    CRect rc(x-tol, y-tol, x+tol, y+tol);
    if (rc.PtInRect(pointTest))
    {
      Invalidate();
      UpdateWindow();
      dc.Rectangle(rc);
      m_enumHit = HIT_POINT;
      m_nHitTest = i;
      return;
    }
  }
  
  if (n > 1)
  {
    const coord_type* p = (coord_type*) pp->GetItemData(0);
    int x1 = int(sizeTotal.cx * p->xcoord / (pDoc->cx-1) + 0.5);
    int y1 = -int(sizeTotal.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
    for (int i = 1; i < n; ++i)
    {
      p = (coord_type*) pp->GetItemData(i);
      int x2 = int(sizeTotal.cx * p->xcoord / (pDoc->cx-1) + 0.5);
      int y2 = -int(sizeTotal.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);

      long x, y;
      if  (DO_INTERSECT == lines_intersect(x1, y1, x2, y2, pointTest.x-tol, pointTest.y-tol, pointTest.x+tol, pointTest.y-tol, &x, &y)
        || DO_INTERSECT == lines_intersect(x1, y1, x2, y2, pointTest.x-tol, pointTest.y+tol, pointTest.x+tol, pointTest.y+tol, &x, &y)
        || DO_INTERSECT == lines_intersect(x1, y1, x2, y2, pointTest.x-tol, pointTest.y-tol, pointTest.x-tol, pointTest.y+tol, &x, &y)
        || DO_INTERSECT == lines_intersect(x1, y1, x2, y2, pointTest.x+tol, pointTest.y-tol, pointTest.x+tol, pointTest.y+tol, &x, &y))
      {
        Invalidate();
        UpdateWindow();
        dc.Rectangle(x1-tol, y1-tol, x1+tol, y1+tol);
        dc.Rectangle(x2-tol, y2-tol, x2+tol, y2+tol);
        m_enumHit = HIT_SEGMENT;
        m_nHitTest = i-1;
        return;
      }
      x1 = x2, y1 = y2;
    }
  }

  if (m_enumHit != HIT_NONE)
  {
    Invalidate();
    UpdateWindow();
    m_enumHit = HIT_NONE;
  }
}

void CFieldView::OnMouseMove(UINT nFlags, CPoint point) 
{
  HitCrossSection(point);
  CScrollView::OnMouseMove(nFlags, point);
}

void CFieldView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  // CScrollView::OnLButtonDown(nFlags, point);
  if (m_enumHit != HIT_NONE)
  {
    HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(coord_type));
    ASSERT(hData);

    coord_type* pData = (coord_type*) GlobalLock(hData);
    ASSERT(pData);

    DWORD eff;
    if (m_enumHit == HIT_POINT)
    {
      coord_type* p = (coord_type*) GetCrossSection()->m_ctrlPoints.GetItemData(m_nHitTest);
      *pData = *p;
      eff = DROPEFFECT_COPY | DROPEFFECT_MOVE;
    }
    else
    {
      CClientDC dc(this);
      OnPrepareDC(&dc);
      dc.DPtoLP(&point);
      CSize sizeTotal = GetTotalSize();
      const CActField* pDoc = GetDocument();
      ASSERT_VALID(pDoc);

      pData->xcoord = (double) (pDoc->cx-1) * point.x / sizeTotal.cx;
      pData->ycoord = (double) (pDoc->cy-1) * (sizeTotal.cy + point.y) / sizeTotal.cy;
      eff = DROPEFFECT_MOVE;
    }
    GlobalUnlock(hData);

    COleDataSource src;
    src.CacheGlobalData(s_nDragPointFormat, hData);
    DROPEFFECT de = src.DoDragDrop(eff);
  }
}

DROPEFFECT CFieldView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
  // CScrollView::OnDragEnter(pDataObject, dwKeyState, point);
  if (!pDataObject->IsDataAvailable(s_nDragPointFormat))
    return DROPEFFECT_NONE;

  return (dwKeyState & MK_CONTROL? DROPEFFECT_COPY: DROPEFFECT_MOVE);
}

void CFieldView::OnDragLeave() 
{
  CScrollView::OnDragLeave();
  Invalidate();
  UpdateWindow();
}

DROPEFFECT CFieldView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint pt) 
{
  // CScrollView::OnDragOver(pDataObject, dwKeyState, pt);
  if (!pDataObject->IsDataAvailable(s_nDragPointFormat))
    return DROPEFFECT_NONE;

  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&pt);
  CSize tot = GetTotalSize();
  CRect rc(0, -tot.cy, tot.cx, 0);
  if (!rc.PtInRect(pt))
    return DROPEFFECT_NONE;

  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  Invalidate();
  UpdateWindow();

  CPen pn(PS_DOT, 0, RGB(255,255,255)), *pPn = dc.SelectObject(&pn);
  DROPEFFECT eff = (m_enumHit == HIT_POINT && (dwKeyState & MK_CONTROL) != 0? 
    DROPEFFECT_COPY: DROPEFFECT_MOVE);

  HANDLE hData = pDataObject->GetGlobalData(s_nDragPointFormat);
  coord_type *pData = (coord_type*) GlobalLock(hData);
  int x = int(tot.cx * pData->xcoord / (pDoc->cx-1) + 0.5);
  int y = -int(tot.cy * (1 - pData->ycoord / (pDoc->cy-1)) + 0.5);
  int dx = pt.x - x, dy = pt.y - y;
  GlobalUnlock(hData);

  CListCtrl* pp = &GetCrossSection()->m_ctrlPoints;
  if (m_enumHit == HIT_SEGMENT)
  {
    if (m_nHitTest > 0)
    {
      coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest-1);
      x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
      y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
      dc.MoveTo(x, y);

      p = (coord_type*) pp->GetItemData(m_nHitTest);
      x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5) + dx;
      y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5) + dy;
      dc.LineTo(x, y);
    }
    else
    {
      coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest);
      x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5) + dx;
      y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5) + dy;
      dc.MoveTo(x, y);
    }

    coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest+1);
    x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5) + dx;
    y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5) + dy;
    dc.LineTo(x, y);

    if (m_nHitTest < pp->GetItemCount()-2)
    {
      p = (coord_type*) pp->GetItemData(m_nHitTest+2);
      x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
      y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
      dc.LineTo(x, y);
    }
  }
  else if (dwKeyState & MK_CONTROL)
  {
    if (m_nHitTest > 0)
    {
      coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest-1);
      x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
      y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
      dc.MoveTo(x, y);

      dc.LineTo(pt);
    }
    else
    {
      dc.MoveTo(pt);
    }
    coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest);
    x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
    y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
    dc.LineTo(x, y);
  }
  else
  {
    if (m_nHitTest > 0)
    {
      coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest-1);
      x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
      y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
      dc.MoveTo(x, y);

      dc.LineTo(pt);
    }
    else
    {
      dc.MoveTo(pt);
    }
    if (m_nHitTest < pp->GetItemCount()-1)
    {
      coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest+1);
      x = int(tot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
      y = -int(tot.cy * (1 - p->ycoord / (pDoc->cy-1)) + 0.5);
      dc.LineTo(x, y);
    }
  }
  if (pPn) dc.SelectObject(pPn);
  return eff;
}

BOOL CFieldView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
  // CScrollView::OnDrop(pDataObject, dropEffect, point);
  if (!pDataObject->IsDataAvailable(s_nDragPointFormat))
    return FALSE;

  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&point);
  CSize tot = GetTotalSize();
  CRect rc(0, -tot.cy, tot.cx, 0);
  if (!rc.PtInRect(point))
    return FALSE;

  CActField* pDoc = GetDocument();
  HANDLE hData = pDataObject->GetGlobalData(s_nDragPointFormat);
  coord_type *pData = (coord_type*) GlobalLock(hData);
  double dx = (double) (pDoc->cx-1) * point.x / tot.cx - pData->xcoord;
  double dy = (double) (pDoc->cy-1) * (tot.cy + point.y) / tot.cy - pData->ycoord;
  GlobalUnlock(hData);

  CListCtrl* pp = &GetCrossSection()->m_ctrlPoints;
  if (m_enumHit == HIT_SEGMENT)
  {
    coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest);
    p->xcoord += dx;
    p->ycoord += dy;

    p = (coord_type*) pp->GetItemData(m_nHitTest+1);
    p->xcoord += dx;
    p->ycoord += dy;
    UpdateCtrlPoint(m_nHitTest+1);
  }
  else if (dropEffect == DROPEFFECT_COPY)
  {
    coord_type* p_old = (coord_type*) pp->GetItemData(m_nHitTest);
    coord_type* p_new = new coord_type;
    p_new->xcoord = p_old->xcoord + dx;
    p_new->ycoord = p_old->ycoord + dy;
    pp->InsertItem(m_nHitTest, "");
    pp->SetItemData(m_nHitTest, (DWORD) p_new);
  }
  else
  {
    coord_type* p = (coord_type*) pp->GetItemData(m_nHitTest);
    p->xcoord += dx;
    p->ycoord += dy;
  }
  UpdateCtrlPoint(m_nHitTest);
  GetDocument()->UpdateAllViews(0);

  return TRUE;
}

void CFieldView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  /*
  if (lHint == CResDialog::IDD || lHint == CCumulativeS::IDD)
  {
  }
  */
  m_wndColorPalette.Invalidate();
  Invalidate();
}

void CFieldView::OnFieldQuerywellbore()
{
  GetDocument()->ExcelOutput(m_nBoreSelected);

#if 0
  long bore = m_nBoreSelected;
  CActField* pDoc = GetDocument();
  CFieldDoc::LogtrackItem tt[1];
  int nt = pDoc->Logtrack(bore, "SP", 1, tt);
  if (nt > 0)
  {
    afxDump << tt[0].num_data << "\n";
  }
  else
  {
    afxDump << "0\n";
  }
  FreeLogtrack(nt, tt);

  nt = pDoc->Logtrack(bore, "IL", 1, tt);
  if (nt > 0)
  {
    afxDump << tt[0].num_data << "\n";
  }
  else
  {
    afxDump << "0\n";
  }
  FreeLogtrack(nt, tt);

  nt = pDoc->Logtrack(bore, "PZ", 1, tt);
  if (nt > 0)
  {
    afxDump << tt[0].num_data << "\n";
  }
  else
  {
    afxDump << "0\n";
  }
  FreeLogtrack(nt, tt);
#endif
}
