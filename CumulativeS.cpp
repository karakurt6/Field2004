// CumulativeS.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include <assert.h>
#include <fstream>
#include <iomanip>
#include "field.h"
#include "CumulativeS.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "FieldForm.h"
#include ".\cumulatives.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int MIN_TILES = 5;
const int MAX_TILES = 10;

const color_type color_oil = { 1.0, 0.5, 0.0};
const color_type color_gas = { 1.0, 1.0, 0.0};
const color_type color_wat = { 0.0, 0.0, 1.0};

const coord_type coord_oil = { 100.0, 100.0 };
const coord_type coord_gas = { 300.0, 400.0 };
const coord_type coord_wat = { 400.0, 100.0 };

double clamp(double x, double x1, double x2)
{
	if (x < x1) return x1;
	if (x > x2) return x2;
	return x;
}

void trilinear_to_cartesian(double x1, double y1, double x2, double y2, double x3, double y3, \
	double alpha, double betta, double gamma, double* x, double* y)
{
	double c1 = x1 - x2, c2 = y1 - y2, c = hypot(c1, c2);
	double a1 = x2 - x3, a2 = y2 - y3, a = hypot(a1, a2);
	double k = -(c2 * (x3 - x1) - c1 * (y3 - y1)) / (a * alpha + hypot(x3 - x1, y3 - y1) * betta + c * gamma);
	a1 /= a, a2 /= a;
	c1 /= c, c2 /= c;
	double s = (k * alpha - k * gamma * (a2 * c2 + a1 * c1) \
		+ a2 * (x1 - x2) + a1 * (y2 - y1)) / (c2 * a1 - c1 * a2);
	*x = x1 + s * c1 - k * gamma * c2;
	*y = y1 + s * c2 + k * gamma * c1;
}

bool cartesian_to_trilinear(double x1, double y1, double x2, double y2, double x3, double y3, \
	double x, double y, double *alpha, double *betta, double *gamma)
{
	double a = hypot(x3 - x2, y3 - y2);
	double b = hypot(x1 - x3, y1 - y3);
	double c = hypot(x2 - x1, y2 - y1);
	double t1 = ((x - x2) * (y3 - y2) - (y - y2) * (x3 - x2)) / a;
	double t2 = ((x - x3) * (y1 - y3) - (y - y3) * (x1 - x3)) / b;
	double t3 = ((x - x1) * (y2 - y1) - (y - y1) * (x2 - x1)) / c;
	double tt = ((x1 - x2) * (y3 - y2) - (y1 - y2) * (x3 - x2)) / a;
	*alpha = t1 / tt;
	*betta = t2 / tt;
	*gamma = t3 / tt;
	return (t1 >= 0.0 && t2 >= 0.0 && t3 >= 0.0);
}

bool cartesian_to_index(double x1, double y1, double x2, double y2, double x3, double y3, \
	double x, double y, int size, int *index)
{
	double e11 = x2 - x1, e12 = y2 - y1, e1 = hypot(e11, e12);
	double e21 = x3 - x1, e22 = y3 - y1, e2 = hypot(e21, e22);
	e11 /= e1, e12 /= e1;
	e21 /= e2, e22 /= e2;

	double sx = size / e1, sy = size / e2;
	double xx = x - x1;
	double yy = y - y1;
	double ee = e11 * e22 - e21 * e12;

	double tol = 1.0e-5;
	double row = (e22 * xx - e21 * yy) * sx / ee;
  double col = (e11 * yy - e12 * xx) * sy / ee;
	if (-tol < row && row < 0.0) row = 0.0;
	if (-tol < col && col < 0.0) col = 0.0;
	if (size <= row && row < size+tol) row = size-1;
	if (size <= col && col < size+tol) col = size-1;
	if (row+col >= size)
	{
		row = floor(row);
		col = floor(col);
	}
	if (0.0 <= row && 0.0 <= col && row+col <= size)
	{
		int n_col = (int) col;
		int n_row = (int) row;
  	double delta = col - n_col + row - n_row;
  	*index = 2 * n_row * size + 2 * n_col - n_row * n_row + (int) delta;
  	return true;
	}
	{
  	double t1, t2, t3;
  	cartesian_to_trilinear(x1, y1, x2, y2, x3, y3, x, y, &t1, &t2, &t3);
  	if (t1 < 0.0) t1 = 0.0;
  	if (t2 < 0.0) t2 = 0.0;
  	if (t3 < 0.0) t3 = 0.0;
  	trilinear_to_cartesian(x1, y1, x2, y2, x3, y3, t1, t2, t3, &x, &y);

  	xx = x - x1;
  	yy = y - y1;
  	double row = (e22 * xx - e21 * yy) * sx / ee;
    double col = (e11 * yy - e12 * xx) * sy / ee;
  	if (0.0 <= row && 0.0 <= col && row+col <= size)
  	{
   		int n_col = (int) col;
   		int n_row = (int) row;
     	double delta = col - n_col + row - n_row;
    	if (row+col == size) delta = 0.0;
     	*index = 2 * n_row * size + 2 * n_col - n_row * n_row + (int) delta;
  	}
  	else
  	{
  		*index = -1;
  	}
	}
	return false;
}

void fill_vertex_colors(const coord_type* p1, const coord_type* p2, const coord_type* p3, \
	const color_type* s1, const color_type* s2, const color_type* s3, int n, color_type* s)
{
	for (int j = 1; j <= n; ++j)
	{
		for (int i = 1; i <= n-j+1; ++i)
		{
			double x1 = p1->xcoord + (j-1) * (p2->xcoord - p1->xcoord) / n \
				+ (i-1) * (p3->xcoord - p1->xcoord) / n;
			double y1 = p1->ycoord + (j-1) * (p2->ycoord - p1->ycoord) / n \
				+ (i-1) * (p3->ycoord - p1->ycoord) / n;
			double x2 = p1->xcoord + (j-1) * (p2->xcoord - p1->xcoord) / n \
				+ i * (p3->xcoord - p1->xcoord) / n;
			double y2 = p1->ycoord + (j-1) * (p2->ycoord - p1->ycoord) / n \
				+ i * (p3->ycoord - p1->ycoord) / n;
			double x3 = p1->xcoord + j * (p2->xcoord - p1->xcoord) / n \
				+ (i-1) * (p3->xcoord - p1->xcoord) / n;
			double y3 = p1->ycoord + j * (p2->ycoord - p1->ycoord) / n \
				+ (i-1) * (p3->ycoord - p1->ycoord) / n;

			{
				double x = (x1 + x2 + x3) / 3.0;
				double y = (y1 + y2 + y3) / 3.0;

				double t1, t2, t3;
				cartesian_to_trilinear(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, x, y, &t1, &t2, &t3);

				s->r = clamp(s1->r * t1 + s2->r * t2 + s3->r * t3, 0.0, 1.0);
				s->g = clamp(s1->g * t1 + s2->g * t2 + s3->g * t3, 0.0, 1.0);
				s->b = clamp(s1->b * t1 + s2->b * t2 + s3->b * t3, 0.0, 1.0);
				++s;
			}

			if (i + j <= n)
			{
  			double x4 = p1->xcoord + j * (p2->xcoord - p1->xcoord) / n \
  				+ i * (p3->xcoord - p1->xcoord) / n;
  			double y4 = p1->ycoord + j * (p2->ycoord - p1->ycoord) / n \
  				+ i * (p3->ycoord - p1->ycoord) / n;

				double x = (x4 + x2 + x3) / 3.0;
				double y = (y4 + y2 + y3) / 3.0;

				double t1, t2, t3;
				cartesian_to_trilinear(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, x, y, &t1, &t2, &t3);

				s->r = clamp(s1->r * t1 + s2->r * t2 + s3->r * t3, 0.0, 1.0);
				s->g = clamp(s1->g * t1 + s2->g * t2 + s3->g * t3, 0.0, 1.0);
				s->b = clamp(s1->b * t1 + s2->b * t2 + s3->b * t3, 0.0, 1.0);
				++s;
			}
		}
	}
}

CActField* CCumulativeS::GetDocument() const
{
  return GetFieldView()->GetDocument();
}

bool CCumulativeS::IsAvailable() const
{
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  const int nn = 20;
  const CActField::DataField* pp[nn];
  int n = pDoc->FieldEnum(nn, pp);
  _ASSERT(n <= nn);
  bool has_g = false, has_o = false, has_c = false;
  for (int i = 0; i < n; ++i)
  {
    if (stricmp(pp[i]->name, "soil") == 0)
      has_o = true;
    if (stricmp(pp[i]->name, "sgas") == 0)
      has_g = true;
		if (stricmp(pp[i]->name, "perm") == 0)
			has_c = true;
  }
  return (has_o && has_g && has_c);
}

/////////////////////////////////////////////////////////////////////////////
// CCumulativeS dialog

CCumulativeS::CCumulativeS(): CDialog(CCumulativeS::IDD, NULL)
, m_nSize(MIN_TILES), m_pColorData(0)
, m_minCollector(0.001f)
, m_maxCollector(10.0f)
{
	//{{AFX_DATA_INIT(CCumulativeS)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
  init = false;
	m_pColorData = new color_type [m_nSize * m_nSize];
	fill_vertex_colors(&coord_oil, &coord_gas, &coord_wat, \
		&color_oil, &color_gas, &color_wat, m_nSize, m_pColorData);
}

CCumulativeS::~CCumulativeS()
{
	delete[] m_pColorData;

	if (init)
	{
    delete[] fn_o;
    delete[] fn_g;
		delete[] fn_c;
    H5Dclose(id_o);
    H5Dclose(id_g);
		H5Dclose(id_c);
	}
}

void CCumulativeS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCumulativeS)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT1, m_nSize);
	DDV_MinMaxInt(pDX, m_nSize, MIN_TILES, MAX_TILES);
	DDX_Control(pDX, IDC_SPIN1, m_wndSpinButton);
	DDX_Text(pDX, IDC_EDIT2, m_minCollector);
	DDX_Text(pDX, IDC_EDIT7, m_maxCollector);
	if (pDX->m_bSaveAndValidate && (m_minCollector <= 0.0 || m_minCollector >= m_maxCollector))
	{
		AfxMessageBox(TEXT("Invalid min/max collector value"));
		pDX->m_idLastControl = IDC_EDIT2;
		pDX->m_bEditLastControl = TRUE;
		pDX->Fail();
	}
}


BEGIN_MESSAGE_MAP(CCumulativeS, CDialog)
	//{{AFX_MSG_MAP(CCumulativeS)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
//	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
//	ON_EN_KILLFOCUS(IDC_EDIT1, OnEnKillfocusEdit1)
ON_WM_LBUTTONDBLCLK()
ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
ON_COMMAND(ID_FILE_SAVE, OnFileSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCumulativeS message handlers

void CCumulativeS::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

  GetFieldForm()->GetDlgItem(IDC_COMBO1)->EnableWindow(!bShow);

  CFieldView* pView = GetFieldView();
  CActField* pDoc = pView->GetDocument();
  if (bShow)
  {
    CWnd* pWnd = &pView->m_wndColorPalette;
    m_bColorPalette = pWnd->IsWindowVisible();
    pWnd->ShowWindow(SW_HIDE);
    pView->m_bColoring = FALSE;

    id_g = H5Dopen(pDoc->hdf5_model, "sgas");
    id_o = H5Dopen(pDoc->hdf5_model, "soil");
		id_c = H5Dopen(pDoc->hdf5_model, "perm");

    int nn = pDoc->cx * pDoc->cy;
    fn_g = new float[nn];
    fn_o = new float[nn];
		fn_c = new float[nn];
    init = true;

    UpdateSelection();
  }
  else if (init)
  {
    CWnd *pWnd = &pView->m_wndColorPalette;
    pWnd->ShowWindow(m_bColorPalette);
    pView->m_bColoring = m_bColorPalette;

    delete[] fn_o;
    delete[] fn_g;
		delete[] fn_c;
    H5Dclose(id_o);
    H5Dclose(id_g);
		H5Dclose(id_c);
    init = false;
  }
  
  pDoc->UpdateAllViews(0, IDD);
}

void CCumulativeS::OnHideWindow()
{
  /*
  if (!init)
    return;

  CFieldForm* pForm = GetFieldForm();
  pForm->GetDlgItem(IDC_COMBO1)->EnableWindow(TRUE);

  CFieldView* pView = GetFieldView();
  CWnd *pWnd = &pView->m_wndColorPalette;
  pWnd->ShowWindow(m_bColorPalette);
  pView->m_bColoring = m_bColorPalette;


  CActField* pDoc = pForm->GetDocument();
  pDoc->UpdateSelection();
  pDoc->UpdateAllViews(0, 1);
  */
}

void CCumulativeS::UpdateSelection()
{
  CActField* pDoc = GetFieldView()->GetDocument();

  hid_t o_space;
  hid_t g_space;
	hid_t c_space;
  hid_t m_space;
  herr_t ret;
  hsize_t start[3];
  hsize_t count[3];
  CWaitCursor ww;

  int nn = pDoc->cx * pDoc->cy;
  o_space = H5Dget_space(id_o);
  g_space = H5Dget_space(id_g);
	c_space = H5Dget_space(id_c);
  start[0] = pDoc->level, start[1] = 0, start[2] = 0;
  count[0] = 1, count[1] = pDoc->cy, count[2] = pDoc->cx;
  ret = H5Sselect_hyperslab(o_space, H5S_SELECT_SET, start, NULL, count, NULL);
  ret = H5Sselect_hyperslab(g_space, H5S_SELECT_SET, start, NULL, count, NULL);
	ret = H5Sselect_hyperslab(c_space, H5S_SELECT_SET, start, NULL, count, NULL);

  count[0] = nn;
  m_space = H5Screate_simple(1, count, NULL);
  H5Dread(id_o, H5T_NATIVE_FLOAT, m_space, o_space, H5P_DEFAULT, fn_o);
  H5Dread(id_g, H5T_NATIVE_FLOAT, m_space, g_space, H5P_DEFAULT, fn_g);
	H5Dread(id_c, H5T_NATIVE_FLOAT, m_space, c_space, H5P_DEFAULT, fn_c);
	// std::transform(fn_c, fn_c+nn, fn_c, std::log10);

  ret = H5Sclose(m_space);
  ret = H5Sclose(o_space);
  ret = H5Sclose(g_space);
	ret = H5Sclose(c_space);

  pDoc->UpdateAllViews(GetFieldForm(), IDD);
}

void CCumulativeS::OnClose() 
{
	CDialog::OnClose();
  OnShowWindow(FALSE, 0);
  // OnHideWindow();
  // OnCancel();
}

void CCumulativeS::OnCancel() 
{
  CDialog::OnCancel();
  OnShowWindow(FALSE, 0);
  // OnHideWindow();
}

void CCumulativeS::LookupColor(double o, double g, double c, color_type* s) const
{
	double min_collector = m_minCollector;
	double max_collector = m_maxCollector;
	if (c < max_collector)
	{
		if (c < min_collector)
		{
			c = min_collector;
		}
		c = log(c/min_collector) / log(max_collector/min_collector);
		COLORREF c1 = m_minColor.GetColor();
		COLORREF c2 = m_maxColor.GetColor();
		s->r = (GetRValue(c1) + (GetRValue(c2) - GetRValue(c1)) * c) / 255.0;
		s->g = (GetGValue(c1) + (GetGValue(c2) - GetGValue(c1)) * c) / 255.0;
		s->b = (GetBValue(c1) + (GetBValue(c2) - GetBValue(c1)) * c) / 255.0;
	}
	else
	{
		const coord_type* p1 = &coord_oil;
		const coord_type* p2 = &coord_gas;
		const coord_type* p3 = &coord_wat;

		double t1 = 0.01 * o;
		double t2 = 0.01 * g;
		double t3 = 1.0 - t1 - t2;
		assert(0.0 <= t1 && t1 <= 1.0);
		assert(0.0 <= t2 && t2 <= 1.0);
		assert(0.0 <= t3 && t3 <= 1.0);

		int k;
		double x, y;
		trilinear_to_cartesian(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
			p3->xcoord, p3->ycoord, t1, t2, t3, &x, &y);
		cartesian_to_index(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
			p3->xcoord, p3->ycoord, x, y, m_nSize, &k);
		assert(0 <= k && k < m_nSize*m_nSize);
		*s = m_pColorData[k];
	}
}

void CCumulativeS::DrawDataNodes(CDC* pDC, const int* pg, const int* vp)
{
  CActField *pDoc = GetFieldView()->GetDocument();
  int cx = pDoc->cx, cy = pDoc->cy;

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
	// const coord_type* p1 = &coord_oil;
	// const coord_type* p2 = &coord_gas;
	// const coord_type* p3 = &coord_wat;
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
      const int nx = 6, ny = 6;
      float o11 = fn_o[n], o21 = fn_o[n+1], o12 = fn_o[n+cx], o22 = fn_o[n+cx+1];
      float g11 = fn_g[n], g21 = fn_g[n+1], g12 = fn_g[n+cx], g22 = fn_g[n+cx+1];
      float c11 = fn_c[n], c21 = fn_c[n+1], c12 = fn_c[n+cx], c22 = fn_c[n+cx+1];
      for (int jj = 0; jj < ny; ++jj)
      {
        rectSample.top = rectCell.top + jj * (rectCell.bottom - rectCell.top) / ny - sizePel.cy;
        rectSample.bottom = rectCell.top + (jj+1) * (rectCell.bottom - rectCell.top) / ny + sizePel.cy;
        float v = (jj+0.5f) / ny;
        for (int ii = 0; ii < nx; ++ii)
        {
          rectSample.left = rectCell.left + ii * (rectCell.right - rectCell.left) / nx - sizePel.cx;
          rectSample.right = rectCell.left + (ii+1) * (rectCell.right - rectCell.left) / nx + sizePel.cx;
          float u = (ii+0.5f) / ny;
          double o = (o11*(1-u)*(1-v)+o21*u*(1-v)+o12*(1-u)*v+o22*u*v);
          double g = (g11*(1-u)*(1-v)+g21*u*(1-v)+g12*(1-u)*v+g22*u*v);
          double c = (c11*(1-u)*(1-v)+c21*u*(1-v)+c12*(1-u)*v+c22*u*v);
					// double t3 = 1.0 - t1 - t2;

					// int k;
					// double x, y;
					// trilinear_to_cartesian(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					//	p3->xcoord, p3->ycoord, t1, t2, t3, &x, &y);
					// cartesian_to_index(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					// 	p3->xcoord, p3->ycoord, x, y, m_nSize, &k);
					// assert(0 <= k && k < m_nSize*m_nSize);
					// const color_type* s = m_pColorData + k;
					color_type s;
					LookupColor(o, g, c, &s);

          // COLORREF cr = RGB(int(2.55 * o + 0.5), int(2.55 * g + 0.5), int(2.55 * (100 - o - g) + 0.5));
					CBrush brushCell(RGB(int(255.0*s.r+0.5), int(255.0*s.g+0.5), int(255.0*s.b+0.5)));
          CBrush *pDefaultBrush = pDC->SelectObject(&brushCell);
					if (pDefaultBrush)
					{
						pDC->Rectangle(&rectSample);
						pDC->SelectObject(pDefaultBrush);
					}
        }
      }
    }
  }
  if (pDefaultPen) pDC->SelectObject(pDefaultPen);
}

void CCumulativeS::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

  CRect rc;
  GetClientRect(&rc);
  rc.DeflateRect(CSize(10, 10));
  rc.bottom -= 50;
  rc.left += 20;
  rc .right -= 20;

  int cx = rc.Width();
  int cy = rc.Height();

	coord_type p1, p2, p3;
	p1.xcoord = rc.left + 10.0;
	p1.ycoord = rc.bottom - 10.0;
	p3.xcoord = rc.right - 10.0;
	p3.ycoord = p1.ycoord;
	p2.xcoord = (p1.xcoord + p3.xcoord) * 0.5;
	p2.ycoord = rc.top + 10.0;

	int n = m_nSize;
	const color_type* s = m_pColorData;
	int mode = dc.SetPolyFillMode(WINDING);
	for (int j = 1; j <= n; ++j)
	{
		for (int i = 1; i <= n-j+1; ++i)
		{
			double x1 = p1.xcoord + (j-1) * (p2.xcoord - p1.xcoord) / n \
				+ (i-1) * (p3.xcoord - p1.xcoord) / n;
			double y1 = p1.ycoord + (j-1) * (p2.ycoord - p1.ycoord) / n \
				+ (i-1) * (p3.ycoord - p1.ycoord) / n;
			double x2 = p1.xcoord + (j-1) * (p2.xcoord - p1.xcoord) / n \
				+ i * (p3.xcoord - p1.xcoord) / n;
			double y2 = p1.ycoord + (j-1) * (p2.ycoord - p1.ycoord) / n \
				+ i * (p3.ycoord - p1.ycoord) / n;
			double x3 = p1.xcoord + j * (p2.xcoord - p1.xcoord) / n \
				+ (i-1) * (p3.xcoord - p1.xcoord) / n;
			double y3 = p1.ycoord + j * (p2.ycoord - p1.ycoord) / n \
				+ (i-1) * (p3.ycoord - p1.ycoord) / n;

			{
				CBrush brush(RGB(int(255.0*s->r+0.5), int(255.0*s->g+0.5), int(255.0*s->b+0.5)));
				CBrush* pBrush = dc.SelectObject(&brush);
				if (pBrush)
				{
					dc.BeginPath();
					dc.MoveTo(int(x1 + 0.5), int(y1 + 0.5));
					dc.LineTo(int(x2 + 0.5), int(y2 + 0.5));
					dc.LineTo(int(x3 + 0.5), int(y3 + 0.5));
					dc.LineTo(int(x1 + 0.5), int(y1 + 0.5));
					dc.EndPath();
					dc.FillPath();
					dc.SelectObject(pBrush);
				}
				++s;
			}

			if (i + j <= n)
			{
  			double x4 = p1.xcoord + j * (p2.xcoord - p1.xcoord) / n \
  				+ i * (p3.xcoord - p1.xcoord) / n;
  			double y4 = p1.ycoord + j * (p2.ycoord - p1.ycoord) / n \
  				+ i * (p3.ycoord - p1.ycoord) / n;

				CBrush brush(RGB(int(255.0*s->r+0.5), int(255.0*s->g+0.5), int(255.0*s->b+0.5)));
				CBrush* pBrush = dc.SelectObject(&brush);
				if (pBrush)
				{
					dc.BeginPath();
					dc.MoveTo(int(x3 + 0.5), int(y3 + 0.5));
					dc.LineTo(int(x2 + 0.5), int(y2 + 0.5));
					dc.LineTo(int(x4 + 0.5), int(y4 + 0.5));
					dc.LineTo(int(x3 + 0.5), int(y3 + 0.5));
					dc.EndPath();
					dc.FillPath();
					dc.SelectObject(pBrush);
				}
				++s;
			}
		}
	}
	dc.SetPolyFillMode(mode);

  mode = dc.SetBkMode(TRANSPARENT);
  dc.SetTextAlign(TA_TOP | TA_RIGHT);
  dc.TextOut(int(p1.xcoord+0.5), int(p1.ycoord+0.5), "So");
  dc.SetTextAlign(TA_BOTTOM | TA_CENTER);
	dc.TextOut(int(p2.xcoord+0.5), int(p2.ycoord+0.5), "Sg");
  dc.SetTextAlign(TA_TOP | TA_LEFT);
	dc.TextOut(int(p3.xcoord+0.5), int(p3.ycoord+0.5), "Sw");
	dc.SetBkMode(mode);
}

BOOL CCumulativeS::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_wndSpinButton.SetRange(MIN_TILES, MAX_TILES);
	m_minColor.AttachButton(IDC_BUTTON1, this);
	m_maxColor.AttachButton(IDC_BUTTON3, this);
	m_minColor.SetColor(RGB(0, 128, 0));
	m_maxColor.SetColor(RGB(100, 225, 100));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCumulativeS::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	// CDialog::OnOK();
	color_type* pColorData = m_pColorData;
	int nSize = m_nSize;
	if (UpdateData())
	{
		m_pColorData = new color_type[m_nSize * m_nSize];
#if 0
		int n = m_nSize;
		color_type* s = m_pColorData;
		const coord_type* p1 = &coord_oil;
		const coord_type* p2 = &coord_gas;
		const coord_type* p3 = &coord_wat;
		for (int j = 1; j <= n; ++j)
		{
			for (int i = 1; i <= n-j+1; ++i)
			{
				double x1 = p1->xcoord + (j-1) * (p2->xcoord - p1->xcoord) / n \
					+ (i-1) * (p3->xcoord - p1->xcoord) / n;
				double y1 = p1->ycoord + (j-1) * (p2->ycoord - p1->ycoord) / n \
					+ (i-1) * (p3->ycoord - p1->ycoord) / n;
				double x2 = p1->xcoord + (j-1) * (p2->xcoord - p1->xcoord) / n \
					+ i * (p3->xcoord - p1->xcoord) / n;
				double y2 = p1->ycoord + (j-1) * (p2->ycoord - p1->ycoord) / n \
					+ i * (p3->ycoord - p1->ycoord) / n;
				double x3 = p1->xcoord + j * (p2->xcoord - p1->xcoord) / n \
					+ (i-1) * (p3->xcoord - p1->xcoord) / n;
				double y3 = p1->ycoord + j * (p2->ycoord - p1->ycoord) / n \
					+ (i-1) * (p3->ycoord - p1->ycoord) / n;

				{
					double x = (x1 + x2 + x3) / 3.0;
					double y = (y1 + y2 + y3) / 3.0;
					int k = -1;
					cartesian_to_index(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
						p3->xcoord, p3->ycoord, x, y, nSize, &k);
					*s++ = pColorData[k];
				}

				if (i + j <= n)
				{
  				double x4 = p1->xcoord + j * (p2->xcoord - p1->xcoord) / n \
  					+ i * (p3->xcoord - p1->xcoord) / n;
  				double y4 = p1->ycoord + j * (p2->ycoord - p1->ycoord) / n \
  					+ i * (p3->ycoord - p1->ycoord) / n;

					double x = (x4 + x2 + x3) / 3.0;
					double y = (y4 + y2 + y3) / 3.0;
					int k = -1;
					cartesian_to_index(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
						p3->xcoord, p3->ycoord, x, y, nSize, &k);
					*s++ = pColorData[k];
				}
			}
		}
#else
		fill_vertex_colors(&coord_oil, &coord_gas, &coord_wat, \
			&color_oil, &color_gas, &color_wat, m_nSize, m_pColorData);
#endif
		delete[] pColorData;
		GetDocument()->UpdateSelection();
		CRect rc;
		GetClientRect(&rc);
		InvalidateRect(rc);
		UpdateWindow();
	}
}

void CCumulativeS::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDblClk(nFlags, point);

  CRect rc;
  GetClientRect(&rc);
  rc.DeflateRect(CSize(10, 10));
  rc.bottom -= 50;
  rc.left += 20;
  rc .right -= 20;

  int cx = rc.Width();
  int cy = rc.Height();

	vertex_type ver(3);
	ver[0].xcoord = rc.left + 10.0;
	ver[0].ycoord = rc.bottom - 10.0;
	ver[2].xcoord = rc.right - 10.0;
	ver[2].ycoord = ver[0].ycoord;
	ver[1].xcoord = (ver[0].xcoord + ver[2].xcoord) * 0.5;
	ver[1].ycoord = rc.top + 10.0;

	coord_type p;
	p.xcoord = (double) point.x;
	p.ycoord = (double) point.y;

	curve_type cur;
	cur.push_back(0);
	cur.push_back(1);
	cur.push_back(2);
	cur.push_back(0);
	if (inside_convex(p.xcoord, p.ycoord, ver, cur))
	{
		int k = -1;
		cartesian_to_index(ver[0].xcoord, ver[0].ycoord, ver[1].xcoord, ver[1].ycoord, \
			ver[2].xcoord, ver[2].ycoord, p.xcoord, p.ycoord, m_nSize, &k);
		color_type* s = m_pColorData + k;
		CColorDialog dlg(RGB(int(255.0*s->r+0.5), int(255.0*s->g+0.5), int(255.0*s->b+0.5)), \
			CC_RGBINIT, this);
		if (IDOK == dlg.DoModal())
		{
			COLORREF cr = dlg.GetColor();
			m_pColorData[k].r = GetRValue(cr) / 255.0;
			m_pColorData[k].g = GetGValue(cr) / 255.0;
			m_pColorData[k].b = GetBValue(cr) / 255.0;
			GetDocument()->UpdateSelection();
			InvalidateRect(rc);
			UpdateWindow();
		}
	}
}

void CCumulativeS::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	OnOK();
	*pResult = 0;
}

std::istream& operator >> (std::istream& in, color_type& s)
{
	return (in >> s.r >> s.g >> s.b);
}

void CCumulativeS::OnFileOpen()
{
	CFileDialog dlg(TRUE, ".txt");
	if (IDOK == dlg.DoModal())
	{
		std::ifstream in(dlg.GetPathName());
		float minCollector = m_minCollector;
		float maxCollector = m_maxCollector;
		COLORREF minColor = m_minColor.GetColor();
		COLORREF maxColor = m_maxColor.GetColor();
		int nSize = m_nSize;
		color_type* pColorData = m_pColorData;
		COLORREF cr;
		in >> m_minCollector >> std::hex >> cr;
		m_minColor.SetColor(cr);
		in >> m_maxCollector >> std::hex >> cr;
		m_maxColor.SetColor(cr);
		if (in >> std::dec >> m_nSize)
		{
			m_pColorData = new color_type[m_nSize * m_nSize];
			std::copy(std::istream_iterator<color_type>(in), \
				std::istream_iterator<color_type>(), m_pColorData);
			if (UpdateData(FALSE))
			{
				GetDocument()->UpdateSelection();
				CRect rc;
				GetClientRect(&rc);
				InvalidateRect(rc);
				UpdateWindow();
				delete[] pColorData;
				return;
			}
			delete[] m_pColorData;
		}
		m_minCollector = minCollector;
		m_maxCollector = maxCollector;
		m_minColor.SetColor(minColor);
		m_maxColor.SetColor(maxColor);
		m_nSize = nSize;
		delete[] pColorData;
	}
}

std::ostream& operator << (std::ostream& out, const color_type& s)
{
	return (out << s.r << ' ' << s.g << ' ' << s.b << std::endl);
}

void CCumulativeS::OnFileSave()
{
	CFileDialog dlg(FALSE, ".txt");
	if (IDOK == dlg.DoModal())
	{
		std::ofstream out(dlg.GetPathName());
		out << m_minCollector << " 0x" << std::hex << std::setfill('0') << std::setw(8) << m_minColor.GetColor() << std::endl;
		out << m_maxCollector << " 0x" << std::hex << std::setfill('0') << std::setw(8) << m_maxColor.GetColor() << std::endl;
		out << m_nSize << std::endl;
		std::copy(m_pColorData, m_pColorData + m_nSize*m_nSize, std::ostream_iterator<color_type>(out));
	}
}
